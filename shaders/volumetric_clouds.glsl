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
#define CLOUD_INNER_RADIUS    (EARTH_RADIUS + 2000.0)
#define CLOUD_OUTER_RADIUS    (EARTH_RADIUS + 5000.0)
#define Hr                    7.994e3
#define Hm                    1.2e3
// Unit: 1 / meters
#define BetaR                 vec3(5.8e-6, 13.5e-6, 33.1e-6)
#define BetaM                 vec3(21e-6)

//#define EYE_HEIGHT            (EARTH_RADIUS + 1.84)
#define EYE_HEIGHT            (CLOUD_INNER_RADIUS + 2200.0)
//#define EYE_HEIGHT            (CLOUD_INNER_RADIUS - 1.0)

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

bool isInvalidHit(hit_t hit) {
    return hit.t < 0.0 || hit.t > max_dist;
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
    const vec2 attack = vec2(0.1, 0.2);
    const vec2 decay = vec2(0.4, 0.5);
    if (f < attack.x) {
        return 0.0;
    } else if (f < attack.y) {
        return remap(f, attack.x, attack.y, 0.0, 1.0);
    } else if (f < decay.x) {
        return 1.0;
    } else if (f < decay.y) {
        return remap(f, decay.x, decay.y, 1.0, 0.0);
    } else {
        return 0.0;
    }
}

vec4 sampleWeather(vec3 wPos) {
    // #todo: User control
    const float WIND_SPEED = 0.00002;
    const float SCALE = 50.0;
    wPos.x += getWorldTime() * EARTH_RADIUS * WIND_SPEED;
    vec2 uv = vec2(0.5) + (SCALE * wPos.xz / EARTH_RADIUS);
    vec4 data = texture(weatherMap, uv);
    data.x = max(0.0, data.x - 0.5);
    return data;
}

// #todo: Replace with a packed texture
float sampleCloudShape(vec3 wPos) {
    // R: Perlin-Worley noise
    // G,B,A: Worley noises at increasing frequencies
    vec4 noises = texture(shapeNoise, 10.0 * wPos / EARTH_RADIUS);
    
    float lowFreqFBM = dot(vec3(0.625, 0.25, 0.125), noises.yzw);
    float baseCloud = noises.x;

    float value = remap(baseCloud, -(1.0 - lowFreqFBM), 1.0, 0.0, 1.0);
    return clamp(value, 0.0, 1.0);
}

// #todo: Replace with a packed texture
float sampleCloudErosion(vec3 wPos) {
    vec3 noises = texture(shapeNoise, 800.0 * wPos / EARTH_RADIUS).xyz;
    return dot(vec3(0.625, 0.25, 0.125), noises);
}

float sampleCloud(vec3 P) {
    vec4 weatherData = sampleWeather(P);
    float cloudCoverage = weatherData.x;

    float baseCloud = sampleCloudShape(P);
    float heightFraction = getCumulusGradient(P);
    baseCloud *= heightFraction;

    float baseCloudWithCoverage = remap(baseCloud, cloudCoverage, 1.0, 0.0, 1.0);
    baseCloudWithCoverage *= cloudCoverage;
    
    // #todo: Apply detail noise
    float erosion = sampleCloudErosion(P);
    float erosionModifier = mix(erosion, 1.0 - erosion, clamp(heightFraction * 10.0, 0.0, 1.0));
	float finalCloud = baseCloudWithCoverage - erosionModifier * (1.0 - baseCloudWithCoverage);
	finalCloud = remap(finalCloud * 2.0, erosionModifier * 0.2, 1.0, 0.0, 1.0);

    return clamp(finalCloud, 0.0, 1.0);
}

// #todo: for debug
bool isMinus(vec3 v) {
    return v.x < 0.0 || v.y < 0.0 || v.z < 0.0;
}

// #todo: This is a total mess
vec3 scene(ray_t camera, vec3 sunDir, vec2 uv)
{
    float cameraHeight = camera.origin.y;
    float totalRayMarchLength = 0.0;
    vec3 raymarchStartPos = vec3(0.0);

    hit_t hit1 = no_hit;
    hit_t hit2 = no_hit;
    intersect_sphere(camera, cloudInnerSphere, hit1);
    intersect_sphere(camera, cloudOuterSphere, hit2);
    if (cameraHeight < CLOUD_INNER_RADIUS) {
        raymarchStartPos = hit1.origin;
        totalRayMarchLength = hit2.t - hit1.t;
    } else if (cameraHeight < CLOUD_OUTER_RADIUS) {
        // DEBUG
        if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
            return vec3(1.0, 0.0, 0.0);
        }
        raymarchStartPos = camera.origin;
        totalRayMarchLength = hit1.t < 0.0 ? hit2.t : hit2.t < 0.0 ? hit1.t : min(hit1.t, hit2.t);
    } else {
        if (isInvalidHit(hit2)) {
            // There's nothing above cloud layer
            return vec3(0.0, 0.0, 0.0);
        }
        raymarchStartPos = hit2.origin;
        totalRayMarchLength = hit1.t - hit2.t;
    }

    // DEBUG: intersection test failed
    if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
        return vec3(1.0, 0.0, 0.0);
    }

    vec3 result = vec3(0.0);
    bool isGround = false;

    bool useDebugColor = false;

    const int numSteps = 64;
    const int inscatSteps = 6;
    
    float cosTheta = dot(camera.direction, -sunDir);
    vec3 T = vec3(1.0); // transmittance
    vec3 L = vec3(0.0); // luminance
    
    vec3 P = raymarchStartPos; // Current sample position
    float seg = totalRayMarchLength / float(numSteps); // dt
    vec3 P_step = camera.direction * seg; // Fixed step size between sample positions

    // (#todo: empty-space optimization)
    for (int i = 0; i < numSteps; ++i)
    {
        float height = P.y - EARTH_RADIUS;
        if (height < 0.0)
        {
            isGround = true;
            break;
        }

        float finalCloud = sampleCloud(P);

        vec3 sigma_a = vec3(finalCloud) * 0.01; // absorption coeff
        vec3 sigma_s = vec3(finalCloud) * 0.3; // scattering coeff
        vec3 TL = vec3(1.0);                   // transmittance(P->Sun)

        hit_t hitL = no_hit;
        ray_t ray2 = ray_t(P, -sunDir);
        intersect_sphere(ray2, cloudOuterSphere, hitL);
        if (isInvalidHit(hitL) == false) {
            float segLight = hitL.t / float(inscatSteps);
            vec3 PL_step = ray2.direction * segLight;
            vec3 PL = P;
            for (int j = 0; j < inscatSteps; ++j) {
                float cloud2 = sampleCloud(PL);
                float sigma_a_L = cloud2 * 0.01;
                TL *= exp(-sigma_a_L * segLight);
                PL += PL_step;
            }
        }
        float scProb = phaseHG(cosTheta, 0.52);
        vec3 sunLuminance = 2.0 * vec3(0.06, 0.06, 0.2); // #todo

        vec3 Lem = 0.008 * vec3(finalCloud); // luminance by emission
        vec3 Lsc = sigma_s * scProb * sunLuminance * TL; // luminance by scattering

        T *= exp(-sigma_a * seg);
        L += (Lem + Lsc) * T * seg;

        if (T.x < 0.01 && T.y < 0.01 && T.z < 0.01) {
            break;
        }

        if (isMinus(Lsc)) {
            useDebugColor = true;
            result = vec3(1.0, 0.0, 0.0);
            break;
        }

        P += P_step;
    }
    
    if (isGround) {
		result = vec3(0.0, 0.0, 0.0);
    } else if (useDebugColor == false) {
        //result = vec3(1.0) - T; // DEBUG: transmittance
        //result = vec3(1.0) - L;
        result = L;
    }

    // DEBUG
    if (isMinus(result) && useDebugColor == false) {
        result = vec3(0.0, 1.0, 1.0);
    }

    return result;
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
