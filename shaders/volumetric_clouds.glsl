#version 450 core

#include "deferred_common.glsl"

// Visualize cloud coverage in weather texture
#define DEBUG_MODE_WEATHER 1
// Visualize raymarching result without applying cloud noise
#define DEBUG_MODE_NO_NOISE 2
// Apply shape noise, but no erosion noise
#define DEBUG_MODE_NO_EROSION 3
// Set to one of values above
#define DEBUG_MODE 0

//layout (std140, binding = 1) uniform UBO_VolumetricClouds {
//	float cameraHeight;
//} ubo;

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0) uniform sampler2D weatherMap;
layout (binding = 1) uniform sampler3D shapeNoise;
layout (binding = 2) uniform sampler3D erosionNoise;
layout (binding = 3, rgba16f) writeonly uniform image2D renderTarget;

//////////////////////////////////////////////////////////////////////////
// Constants
#define PI                    3.14159265359

// Unit: meters
#define EARTH_RADIUS          6.36e6
// #todo: Parametrize cloud range
#define CLOUD_Y_START         2000.0
#define CLOUD_Y_END           6000.0

//#define EYE_HEIGHT            1.84
#define EYE_HEIGHT            (CLOUD_Y_START + 3500.0)
//#define EYE_HEIGHT            (CLOUD_Y_START + 800.0)

// Cone sampling random offsets
uniform vec3 noiseKernel[6u] = vec3[]
(
	vec3( 0.38051305,  0.92453449, -0.02111345),
	vec3(-0.50625799, -0.03590792, -0.86163418),
	vec3(-0.32509218, -0.94557439,  0.01428793),
	vec3( 0.09026238, -0.27376545,  0.95755165),
	vec3( 0.28128598,  0.42443639, -0.86065785),
	vec3(-0.16852403,  0.14748697,  0.97460106)
);

//////////////////////////////////////////////////////////////////////////
// Unused
#define MAGIC_RAYLEIGH        1.0
#define MAGIC_MIE             0.3

#define SUN_DISTANCE          1.496e11
#define SUN_RADIUS            6.9551e8
#define ATMOSPHERE_RADIUS     6.42e6
#define Hr                    7.994e3
#define Hm                    1.2e3
// Unit: 1 / meters
#define BetaR                 vec3(5.8e-6, 13.5e-6, 33.1e-6)
#define BetaM                 vec3(21e-6)

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

sphere_t cloudInnerSphere = sphere_t(vec3(0.0, -EARTH_RADIUS, 0.0), EARTH_RADIUS + CLOUD_Y_START);
sphere_t cloudOuterSphere = sphere_t(vec3(0.0, -EARTH_RADIUS, 0.0), EARTH_RADIUS + CLOUD_Y_END);

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

float powder(float d) {
	return (1.0 - exp(-2.0 * d));
}

float remap(float x, float oldMin, float oldMax, float newMin, float newMax) {
    return newMin + (newMax - newMin) * (x - oldMin) / (oldMax - oldMin);
}

float saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

float getHeightFraction(vec3 wPos) {
    float fraction = (wPos.y - CLOUD_Y_START.x) / (CLOUD_Y_END - CLOUD_Y_START);
    return saturate(fraction);
}
// #todo: Temp function. Replace with a channel of the weather texture
float getCumulusGradient(vec3 wPos) {
    float f = getHeightFraction(wPos);
    const vec2 attack = vec2(0.1, 0.2);
    const vec2 decay = vec2(0.25, 0.9);
    if (f < attack.x) {
        return 0.0;
    } else if (f < attack.y) {
        return remap(f, attack.x, attack.y, 0.0, 1.0);
    } else if (f < decay.x) {
        return 1.0;
    } else if (f < decay.y) {
        return pow(remap(f, decay.x, decay.y, 1.0, 0.0), 2.0);
    } else {
        return 0.0;
    }
}

// #todo: User control
const float WIND_SPEED = 0.2;
const float WEATHER_SCALE = 0.005;
const float CLOUD_SCALE = 0.4; // 0.4
const float CURLINESS = 0.5;

vec4 sampleWeather(vec3 wPos) {
    wPos.x += getWorldTime() * CLOUD_Y_START * WIND_SPEED;
    vec2 uv = vec2(0.5) + (WEATHER_SCALE * wPos.xz / CLOUD_Y_START);
    vec4 data = texture(weatherMap, uv);

    //data.x = max(0.0, data.x - 0.25); // If overall coverage is too high
    return data;
}

// #todo: Replace with a packed texture
vec2 sampleCloudShapeAndErosion(vec3 wPos) {
    vec2 uv = 0.5 + wPos.xz / CLOUD_Y_START;
    vec2 moving_uv = vec2(uv.x + getWorldTime() * WIND_SPEED, uv.y);

    float heightFraction = getHeightFraction(wPos);
    vec3 samplePos = vec3(CLOUD_SCALE * moving_uv, heightFraction);
    vec3 samplePos2 = vec3(CLOUD_SCALE * uv, heightFraction);

    vec2 result = vec2(0.0);

    // R: Perlin-Worley noise
    // G,B,A: Worley noises at increasing frequencies
    vec4 baseNoises = texture(shapeNoise, samplePos);
    float lowFreqFBM = dot(vec3(0.625, 0.25, 0.125), baseNoises.yzw);
    float baseCloud = saturate(baseNoises.x + 0.2);

    vec3 detailNoises = texture(erosionNoise, CURLINESS * samplePos2).xyz;

    result.x = saturate(remap(baseCloud, -(1.0 - lowFreqFBM), 1.0, 0.0, 1.0));
    result.y = dot(vec3(0.625, 0.25, 0.125), detailNoises);
    return result;
}

float sampleCloud(vec3 P) {
    vec4 weatherData = sampleWeather(P);
    float cloudCoverage = weatherData.x;

#if DEBUG_MODE == DEBUG_MODE_NO_NOISE
    return cloudCoverage;
#endif

    vec2 cloudNoiseSamples = sampleCloudShapeAndErosion(P);

    float baseCloud = cloudNoiseSamples.x;
    float heightFraction = getCumulusGradient(P);
    baseCloud *= heightFraction;

    float baseCloudWithCoverage = remap(baseCloud, cloudCoverage, 1.0, 0.0, 1.0);
    baseCloudWithCoverage *= cloudCoverage;

#if DEBUG_MODE == DEBUG_MODE_NO_EROSION
    return saturate(baseCloudWithCoverage);
#endif
    
    float erosion = cloudNoiseSamples.y;
    float erosionModifier = 0.2 * mix(erosion, 1.0 - erosion, saturate(heightFraction * 10.0));
	float finalCloud = baseCloudWithCoverage - erosionModifier * (1.0 - baseCloudWithCoverage);
	finalCloud = remap(finalCloud, erosionModifier * 0.2, 1.0, 0.0, 1.0);

    return saturate(finalCloud);
}

// #todo: for debug
bool isMinus(vec3 v) {
    return v.x < 0.0 || v.y < 0.0 || v.z < 0.0;
}

// #todo: This is a total mess
vec4 scene(ray_t camera, vec3 sunDir, vec2 uv)
{
    float cameraHeight = camera.origin.y;
    float totalRayMarchLength = 0.0;
    vec3 raymarchStartPos = vec3(0.0);

    hit_t hit1 = no_hit;
    hit_t hit2 = no_hit;
    intersect_sphere(camera, cloudInnerSphere, hit1);
    intersect_sphere(camera, cloudOuterSphere, hit2);
    if (cameraHeight < CLOUD_Y_START) {
        raymarchStartPos = hit1.origin;
        totalRayMarchLength = hit2.t - hit1.t;
    } else if (cameraHeight < CLOUD_Y_END) {
        // DEBUG
        if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
            return vec4(1.0, 1.0, 0.0, 1.0);
        }
        raymarchStartPos = camera.origin;
        totalRayMarchLength = hit1.t < 0.0 ? hit2.t : hit2.t < 0.0 ? hit1.t : min(hit1.t, hit2.t);
    } else {
        if (isInvalidHit(hit2)) {
            // There's nothing above cloud layer
            return vec4(0.0, 0.0, 0.0, 1.0);
        }
        raymarchStartPos = hit2.origin;
        totalRayMarchLength = hit1.t - hit2.t;
    }

    // DEBUG: intersection test failed
    if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
        return vec4(1.0, 0.0, 0.0, 1.0);
    }

    vec3 result = vec3(0.0); // final luminance
    bool isGround = false;

    bool useDebugColor = false;

    const int numSteps = 64;
    const int inscatSteps = 6;
    
    float cosTheta = dot(camera.direction, -sunDir);
    float phaseFn = phaseHG(cosTheta, 0.52);

    float T = 1.0; // transmittance
    vec3 L = vec3(0.0); // luminance
    
    vec3 P = raymarchStartPos; // Current sample position
    float seg = totalRayMarchLength / float(numSteps); // dt
    vec3 P_step = camera.direction * seg; // Fixed step size between sample positions

    // (#todo: empty-space optimization)
    // Raymarching
    for (int i = 0; i < numSteps; ++i)
    {
        if (P.y < 0.0)
        {
            isGround = true;
            break;
        }

#if DEBUG_MODE == DEBUG_MODE_WEATHER
        return vec4(vec3(sampleWeather(P).x), 0.0);
#endif

        float cloudDensity = sampleCloud(P);

        float sigma_a = 0.0035; // absorption coeff
        float sigma_s = 0.3; // scattering coeff

        // Raymarch from current position to Sun
        float TL = 1.0; // transmittance(P->Sun)
        {
            hit_t hitL = no_hit;
            ray_t ray2 = ray_t(P, -sunDir);
            intersect_sphere(ray2, cloudOuterSphere, hitL);
            if (isInvalidHit(hitL) == false) {
                // This is too large and will sample densities too far from P
                //float segLight = hitL.t / float(inscatSteps);

                float segLight = seg * 0.01 * float(inscatSteps);
                vec3 PL_step = ray2.direction * segLight;
                vec3 PL = P;

                float coneRadius = 1.0;
                const float CONE_STEP = 1.0/6.0;

                for (int j = 0; j < inscatSteps; ++j) {
#if 0 // Cone sample
                    vec3 samplePos = P + coneRadius * noiseKernel[j] * float(j);

                    TL *= exp(-sigma_a * sampleCloud(samplePos) * segLight);

                    PL += PL_step;
                    coneRadius += CONE_STEP;
#else
                    TL *= exp(-sigma_a * sampleCloud(PL) * segLight);
                    PL += PL_step;
#endif
                }
            }
        }
        vec3 sunLuminance = 20.0 * vec3(0.15, 0.08, 0.06); // #todo

        // luminance by emission
        // #todo: Does cloud emits light?
        // http://www.sc.chula.ac.th/courseware/2309507/Lecture/remote10.htm
        vec3 Lem = 1.0 * vec3(0.13, 0.13, 0.17) * cloudDensity;

        // luminance by scattering
        vec3 Lsc = (cloudDensity * sigma_s) * phaseFn * sunLuminance * TL;

        float dT = exp(-(sigma_a + sigma_s) * cloudDensity * seg);
        T *= dT;

#if 0
        L += dT * (Lem + Lsc) * seg;
#else
        // #todo: Dafuq is this
        vec3 Lsample = dT * (Lem + Lsc) * seg;
        Lsample = mix(vec3(min(0.03, TL)), Lsample, 1.0 - TL) * T;
        L += Lsample;
#endif

        if (T < 0.01) {
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

        // #todo: L looks too artificial and black cloud at T = 0 cannot be distinguished from black atmosphere.
        result = L;
        
        // This hurts too much details of cloud color
        //result = mix(vec3(0.1, 0.1, 0.1), L, T);
    }

    // DEBUG
    if (isMinus(result) && useDebugColor == false) {
        result = vec3(0.0, 1.0, 1.0);
    }

    return vec4(result, T); // luminance and transmittance
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

    vec4 result = scene(eye_ray, sunDir, uv);

    imageStore(renderTarget, currentTexel, result);
}
