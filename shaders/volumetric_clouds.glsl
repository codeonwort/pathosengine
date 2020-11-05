#version 450 core

#include "deferred_common.glsl"

//layout (std140, binding = 1) uniform UBO_VolumetricClouds {
//	vec4 params;
//} ubo;

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0) uniform sampler2D weatherMap;
layout (binding = 1) uniform sampler3D shapeNoise;
layout (binding = 2) uniform sampler3D erosionNoise;
layout (binding = 3, rgba16f) writeonly uniform image2D renderTarget;

//////////////////////////////////////////////////////////////////////////
// Constants
#define MAGIC_RAYLEIGH        1.0
#define MAGIC_MIE             0.3

#define PI                    3.14159265359

// #todo: Represent them as offset from the ground.
//        Physical heights might cause float precision issues.
// Unit: meters
#define SUN_DISTANCE          1.496e11
#define SUN_RADIUS            6.9551e8
#define EARTH_RADIUS          6.36e6
#define ATMOSPHERE_RADIUS     6.42e6
#define CLOUD_INNER_RADIUS    6.362e6
#define CLOUD_OUTER_RADIUS    6.367e6
#define Hr                    7.994e3
#define Hm                    1.2e3
// Unit: 1 / meters
#define BetaR                 vec3(5.8e-6, 13.5e-6, 33.1e-6)
#define BetaM                 vec3(21e-6)

#define EYE_HEIGHT            EARTH_RADIUS
//#define EYE_HEIGHT            (0.5 * (CLOUD_INNER_RADIUS + CLOUD_OUTER_RADIUS))

//////////////////////////////////////////////////////////////////////////
// Ray, sphere code from https://www.shadertoy.com/view/XtBXDw
struct ray_t {
	vec3 origin;
	vec3 direction;
};

struct sphere_t {
	vec3 origin;
	float radius;
};

struct hit_t {
	float t;
	vec3 normal;
	vec3 origin;
};

#define max_dist 1e8
hit_t no_hit = hit_t(
	float(max_dist + 1e1), // 'infinite' distance
	vec3(0., 0., 0.), // normal
	vec3(0., 0., 0.) // origin
);

ray_t get_primary_ray(
	in vec3 cam_local_point,
	inout vec3 cam_origin,
	inout vec3 cam_look_at)
{
	vec3 fwd = normalize(cam_look_at - cam_origin);
	vec3 up = vec3(0, 1, 0);
	vec3 right = cross(up, fwd);
	up = cross(fwd, right);

    ray_t r;
    r.origin = cam_origin;
    r.direction = normalize(fwd + up * cam_local_point.y - right * cam_local_point.x);
	return r;
}

void intersect_sphere(
	in ray_t ray,
	in sphere_t sphere,
	inout hit_t hit
){
	vec3 rc = sphere.origin - ray.origin;
	float radius2 = sphere.radius * sphere.radius;
	float tca = dot(rc, ray.direction);
//	if (tca < 0.) return;

	float d2 = dot(rc, rc) - tca * tca;
	if (d2 > radius2)
		return;

	float thc = sqrt(radius2 - d2);
	float t0 = tca - thc;
	float t1 = tca + thc;

	if (t0 < 0.) t0 = t1;
	if (t0 > hit.t)
		return;

	vec3 impact = ray.origin + ray.direction * t0;

	hit.t = t0;
	hit.origin = impact;
	hit.normal = (impact - sphere.origin) / sphere.radius;
}

//////////////////////////////////////////////////////////////////////////

sphere_t cloudInnerSphere = sphere_t(vec3(0.0), CLOUD_INNER_RADIUS);
sphere_t cloudOuterSphere = sphere_t(vec3(0.0), CLOUD_OUTER_RADIUS);

// Rayleigh
float phaseR(float cosTheta)
{
    return 3.0 / (16.0 * PI) * (1.0 + cosTheta * cosTheta);
}
// Mie
float phaseM(float t)
{
    const float g = 0.76;
    float gg = g * g;
    float tt = t * t;
    float gt = g * t;
    
    float num = 3.0 * (1.0 - gg) * (1.0 + tt);
	float denom = (8.0 * PI) * (2.0 + gg) * pow(1.0 + gg - 2.0 * gt, 1.5);
    return num / denom;
}
// Henyey-Greenstein
float phaseHG(float t, float g) {
    float gg = g * g;
    float num = 1 - gg;
    float denom = 4 * PI * pow(1 + gg - 2 * g * cos(t), 1.5);
    return num / denom;
}
// Cornette-Shank
float phaseCS(float t, float g) {
    float gg = g * g;
    float cost = cos(t);
    float num = 3 * (1 - gg) * (1 + cost * cost);
    float denom = 2 * (2 + gg) * pow(1 + gg - 2 * g * cost, 1.5);
    return num / denom;
}

float remap(float x, float oldMin, float oldMax, float newMin, float newMax) {
    return newMin + (newMax - newMin) * (x - oldMin) / (oldMax - oldMin);
}

float getHeightFraction(vec3 wPos, vec2 cloudMinMax) {
    float fraction = (wPos.y - cloudMinMax.x) / (cloudMinMax.y - cloudMinMax.x);
    return clamp(fraction, 0.0, 1.0);
}
// #todo: Temp function. Replace with a weather texture
float getCumulusGradient(vec3 wPos) {
    float f = getHeightFraction(wPos, vec2(CLOUD_INNER_RADIUS, CLOUD_OUTER_RADIUS));
    const float x1 = 0.2;
    const float x2 = 0.7;
    const float maxVal = 1.0;
    if (f < x1) {
        return remap(f, 0.0, x1, 0.0, maxVal);
    } else if (f < x2) {
        return maxVal;
    } else {
        return remap(f, x2, 1.0, maxVal, 0.0);
    }
}

vec4 sampleWeather(vec3 wPos) {
    wPos.x += getWorldTime() * 100.0;
    vec2 uv = wPos.xz * 0.00001; // #todo: User-friendly control
    return texture(weatherMap, uv);
}

// #todo: Replace with a packed texture
float sampleCloudShape(vec3 wPos) {
    // R: Perlin-Worley noise
    // G,B,A: Worley noises at increasing frequencies
    vec4 noises = texture(shapeNoise, wPos * 0.0002);
    
    float lowFreqFBM = dot(vec3(0.625, 0.25, 0.125), noises.yzw);
    float baseCloud = noises.x;

    float value = remap(baseCloud, -(1.0 - lowFreqFBM), 1.0, 0.0, 1.0);
    return clamp(value, 0.0, 1.0);
}

// #todo: Replace with a packed texture
float sampleCloudErosion(vec3 wPos) {
    vec3 noises = texture(shapeNoise, wPos * 0.0001).xyz;
    return dot(vec3(0.625, 0.25, 0.125), noises);
}

float sampleCloud(vec3 P) {
    vec4 weatherData = sampleWeather(P);
    float cloudCoverage = weatherData.x;

    float baseCloud = sampleCloudShape(P);
    baseCloud -= 0.08 * sampleCloudErosion(P);
    float densityHeightGradient = getCumulusGradient(P);
    baseCloud *= densityHeightGradient;

    float baseCloudWithCoverage = remap(baseCloud, cloudCoverage, 1.0, 0.0, 1.0);
    baseCloudWithCoverage *= cloudCoverage;
        
    // #todo: Apply detail noise
    float finalCloud = baseCloudWithCoverage;

    return finalCloud;
}

// #todo: This is a total mess
vec3 scene(ray_t camera, vec3 sunDir, vec2 uv)
{
    hit_t hit = no_hit;
    intersect_sphere(camera, cloudInnerSphere, hit);

    // DEBUG: intersection test failed
    if (hit.t > max_dist) {
        return vec3(1.0, 0.0, 0.0);
    }

    vec3 result = vec3(0.0);
    bool isGround = false;
    
    const int numSteps = 64;
    const int inscatSteps = 8;
    
    float cosTheta = dot(camera.direction, -sunDir);
    vec3 T = vec3(1.0); // transmittance
    vec3 L = vec3(0.0); // luminance
    
    vec3 P = camera.origin + hit.t * camera.direction; // Current sample position
    float seg = (CLOUD_OUTER_RADIUS - CLOUD_INNER_RADIUS) / float(numSteps); // dt
    vec3 P_step = camera.direction * seg; // Fixed step size between sample positions
    
    // (#todo: empty-space optimization)
    for (int i = 0; i < numSteps; ++i)
    {
        float height = P.y - EYE_HEIGHT;
        if (height < 0.0)
        {
            isGround = true;
            break;
        }

        float finalCloud = sampleCloud(P);

        vec3 sigma_a = vec3(finalCloud) * 0.01; // absorption coeff
        vec3 sigma_s = vec3(finalCloud) * 0.3; // scattering coeff
        vec3 TL = vec3(1.0);                   // transmittance(P->Sun)

        hit_t hit2 = no_hit;
        ray_t ray2 = ray_t(P, -sunDir);
        intersect_sphere(ray2, cloudOuterSphere, hit2);
        float segLight = hit2.t / float(inscatSteps);
        vec3 PL_step = ray2.direction * segLight;
        vec3 PL = P;
        for (int j = 0; j < inscatSteps; ++j) {
            float cloud2 = sampleCloud(PL);
            float sigma_a_L = cloud2 * 0.01;
            TL *= exp(-sigma_a_L * segLight);
            PL += PL_step;
        }
        float scProb = phaseHG(cosTheta, 0.76);
        vec3 sunLuminance = 0.5 * vec3(1.0, 0.7, 0.4); // #todo

        vec3 Lem = 0.005 * vec3(finalCloud); // luminance by emission
        vec3 Lsc = sigma_s * scProb * sunLuminance * TL; // luminance by scattering

        T *= exp(-sigma_a * seg);
        L += (Lem + Lsc) * T * seg;

        if (T.x < 0.01 && T.y < 0.01 && T.z < 0.01) {
            break;
        }
        
        P += P_step;
    }
    
    // Just magic number
    if (isGround) {
		result = vec3(0.0);
    } else {
        result = vec3(1.0) - T; // DEBUG: transmittance
        //result = vec3(1.0) - L;
        //result = L;
    }

    // #todo: Minus value near horizon?
    return max(result, vec3(0.0));
}

vec3 getViewDirection(vec2 uv) {
	vec3 P = vec3(2.0 * uv - 1.0, 0.0);
    P.x *= uboPerFrame.screenResolution.x / uboPerFrame.screenResolution.y;
    P.z = -(1.0 / tan(uboPerFrame.zRange.z * 0.5));
	P = normalize(P);
    
    mat3 camera_transform = mat3(uboPerFrame.inverseViewTransform);
	vec3 ray_forward = camera_transform * P;
    
    return ray_forward;
}

void main() {
    ivec2 sceneSize = imageSize(renderTarget);
    if (gl_GlobalInvocationID.x >= sceneSize.x || gl_GlobalInvocationID.y >= sceneSize.y) {
		return;
	}

    ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(currentTexel) / vec2(sceneSize);

	ray_t eye_ray;
    eye_ray.origin = vec3(0.0, EYE_HEIGHT, 0.0);
	eye_ray.direction = getViewDirection(uv);
    
	vec3 sunDir = uboPerFrame.directionalLights[0].direction;

    vec4 result = vec4(scene(eye_ray, sunDir, uv), 1.0);

    imageStore(renderTarget, currentTexel, result);
}
