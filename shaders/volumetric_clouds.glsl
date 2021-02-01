#version 450 core

#include "deferred_common.glsl"

// #todo-cloud: wip
#define TEMPORAL_REPROJECTION 0

// Visualize cloud coverage in weather texture
#define DEBUG_MODE_WEATHER 1
// Visualize raymarching result without applying cloud noise
#define DEBUG_MODE_NO_NOISE 2
// Apply shape noise, but no erosion noise
#define DEBUG_MODE_NO_EROSION 3

#define DEBUG_MODE 0

#define DEBUG_TRANSMITTANCE 0

layout (std140, binding = 1) uniform UBO_VolumetricCloud {
    float earthRadius;    // in meters
    float cloudLayerMinY; // in meters
    float cloudLayerMaxY; // in meters
    float windSpeedX;

    float windSpeedZ;
    float weatherScale;
    float cloudScale;
    float cloudCurliness;

    uint  frameCounter;
} uboCloud;

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0) uniform sampler2D sceneDepth;
layout (binding = 1) uniform sampler2D weatherMap;
layout (binding = 2) uniform sampler3D shapeNoise;
layout (binding = 3) uniform sampler3D erosionNoise;
layout (binding = 4) uniform sampler2D reprojectionHistory; // cloud RT of prev frame
layout (binding = 5, rgba16f) writeonly uniform image2D renderTarget;

//////////////////////////////////////////////////////////////////////////
// Constants
#define PI                    3.14159265359

// Cone sampling random offsets
const vec3 noiseKernel[6u] = vec3[]
(
	vec3( 0.38051305,  0.92453449, -0.02111345),
	vec3(-0.50625799, -0.03590792, -0.86163418),
	vec3(-0.32509218, -0.94557439,  0.01428793),
	vec3( 0.09026238, -0.27376545,  0.95755165),
	vec3( 0.28128598,  0.42443639, -0.86065785),
	vec3(-0.16852403,  0.14748697,  0.97460106)
);

const uint bayerPattern[16u] = uint[]
(
    0, 8, 2, 10,
    12, 4, 14, 6,
    3, 11, 1, 9,
    15, 7, 13, 5
);

//////////////////////////////////////////////////////////////////////////
// Uniform getters

float getEarthRadius() { return uboCloud.earthRadius; }
float getCloudLayerMin() { return uboCloud.cloudLayerMinY; }
float getCloudLayerMax() { return uboCloud.cloudLayerMaxY; }
vec2 getWindSpeed() { return vec2(uboCloud.windSpeedX, uboCloud.windSpeedZ); }
float getWeatherScale() { return uboCloud.weatherScale; }
float getCloudScale() { return uboCloud.cloudScale; }
float getCloudCurliness() { return uboCloud.cloudCurliness; }

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

void intersect_sphere(
	in ray_t ray,
	in sphere_t sphere,
	inout hit_t hit
){
// Original code from shadertoy
#if 1
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
// Ray Tracing Gems - Chapter 7
// Precision Improvements for Ray/Sphere Intersection
// -> Well, no visual differences. Let's use the original code.
#else
    vec3 d = ray.direction;
    vec3 f = ray.origin - sphere.origin;
    float r = sphere.radius;
    float a = dot(d, d);
    float b = -dot(f, d);
    vec3 X = f + (b/a)*d;
    float det = r*r - dot(X,X);
    
    if (det < 0.0) return;

    float c = dot(f,f) - r*r;
    float q = b + sign(b) * sqrt(a * det);
    float t0 = c/q;
    float t1 = q/a;
    float t = t0 < 0.0 ? t1 : t1 < 0.0 ? t0 : min(t0, t1);

    hit.t = t;
    hit.origin = ray.origin + t * ray.direction;
    hit.normal = (hit.origin - sphere.origin) / sphere.radius;
#endif
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
    float fraction = (wPos.y - getCloudLayerMin()) / (getCloudLayerMax() - getCloudLayerMin());
    return saturate(fraction);
}

// Cloud types height density gradients
#define STRATUS_GRADIENT vec4(0.0, 0.1, 0.2, 0.3)
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)
#define CUMULUS_GRADIENT vec4(0.00, 0.1625, 0.88, 0.98)
float getDensityForCloud(float heightFraction, float cloudType)
{
	float stratusFactor = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
	float stratoCumulusFactor = 1.0 - abs(cloudType - 0.5) * 2.0;
	float cumulusFactor = clamp(cloudType - 0.5, 0.0, 1.0) * 2.0;

	vec4 baseGradient = stratusFactor * STRATUS_GRADIENT + stratoCumulusFactor * STRATOCUMULUS_GRADIENT + cumulusFactor * CUMULUS_GRADIENT;

	// gradicent computation (see Siggraph 2017 Nubis-Decima talk)
	//return remap(heightFraction, baseGradient.x, baseGradient.y, 0.0, 1.0) * remap(heightFraction, baseGradient.z, baseGradient.w, 1.0, 0.0);
	return smoothstep(baseGradient.x, baseGradient.y, heightFraction) - smoothstep(baseGradient.z, baseGradient.w, heightFraction);
}

vec4 sampleWeather(vec3 wPos) {
    wPos.xz += getWorldTime() * getCloudLayerMin() * getWindSpeed();
    vec2 uv = vec2(0.5) + (getWeatherScale() * wPos.xz / getCloudLayerMin());
    vec4 data = textureLod(weatherMap, uv, 0);

    data.x = max(0.0, data.x - 0.1); // If overall coverage is too high
    return data;
}

// #todo: Replace with a packed texture
vec2 sampleCloudShapeAndErosion(vec3 wPos, float lod) {
    vec2 uv = 0.5 + wPos.xz / getCloudLayerMin();
    vec2 moving_uv = uv + getWorldTime() * getWindSpeed();

    float heightFraction = getHeightFraction(wPos);
    vec3 samplePos = vec3(getCloudScale() * moving_uv, heightFraction);
    vec3 samplePos2 = vec3(getCloudScale() * uv, heightFraction);

    vec2 result = vec2(0.0);

    // R: Perlin-Worley noise
    // G,B,A: Worley noises at increasing frequencies
    vec4 baseNoises = textureLod(shapeNoise, samplePos, lod);
    float lowFreqFBM = dot(vec3(0.625, 0.25, 0.125), baseNoises.yzw);

    // #todo: baseCloud values are too high
    float baseCloud = baseNoises.x - 0.92;

    vec3 detailNoises = textureLod(erosionNoise, getCloudCurliness() * samplePos2, lod).xyz;

    result.x = saturate(remap(baseCloud, -(1.0 - lowFreqFBM), 1.0, 0.0, 1.0));
    result.y = dot(vec3(0.625, 0.25, 0.125), detailNoises);
    return result;
}

float sampleCloud(vec3 P, float lod) {
    vec4 weatherData = sampleWeather(P);

    float cloudCoverage = weatherData.x;

#if DEBUG_MODE == DEBUG_MODE_NO_NOISE || DEBUG_MODE == DEBUG_MODE_WEATHER
    return cloudCoverage * getDensityForCloud(getHeightFraction(P), 1.0);
#endif

    vec2 cloudNoiseSamples = sampleCloudShapeAndErosion(P, lod);

    float baseCloud = cloudNoiseSamples.x;
    float heightFraction = getHeightFraction(P);

    float density = getDensityForCloud(heightFraction, 1.0);
    baseCloud *= density / heightFraction;

    float baseCloudWithCoverage = remap(baseCloud, cloudCoverage, 1.0, 0.0, 1.0);
    baseCloudWithCoverage *= cloudCoverage;

#if DEBUG_MODE == DEBUG_MODE_NO_EROSION
    return saturate(baseCloudWithCoverage);
#endif

    float erosion = cloudNoiseSamples.y;
#if 0
    // Erosion is supposed to add small details to the basic shape of cloud,
    // but it decreases cloud coverage too much.
    float erosionModifier = mix(erosion, 1.0 - erosion, saturate(heightFraction * 10.0));
    float finalCloud = baseCloudWithCoverage - erosionModifier * (1.0 - baseCloudWithCoverage);
    finalCloud = remap(finalCloud, erosionModifier * 0.2, 1.0, 0.0, 1.0);
#else
    float erosionModifier = 0.01 * erosion * heightFraction;
	float finalCloud = baseCloudWithCoverage - erosionModifier;
    finalCloud = remap(finalCloud, erosionModifier, 1.0, 0.0, 1.0);
#endif

    return saturate(finalCloud);
}

// #todo: for debug
bool isMinus(vec3 v) {
    return v.x < 0.0 || v.y < 0.0 || v.z < 0.0;
}

// #todo: This is a total mess
vec4 scene(ray_t camera, vec3 sunDir, vec2 uv)
{
    sphere_t cloudInnerSphere = sphere_t(vec3(0.0, -getEarthRadius(), 0.0), getEarthRadius() + getCloudLayerMin());
    sphere_t cloudOuterSphere = sphere_t(vec3(0.0, -getEarthRadius(), 0.0), getEarthRadius() + getCloudLayerMax());

    float cameraHeight = camera.origin.y;
    float totalRayMarchLength = 0.0;
    vec3 raymarchStartPos = vec3(0.0);

    hit_t hit1 = no_hit;
    hit_t hit2 = no_hit;
    intersect_sphere(camera, cloudInnerSphere, hit1);
    intersect_sphere(camera, cloudOuterSphere, hit2);
    if (cameraHeight < getCloudLayerMin()) {
        raymarchStartPos = hit1.origin;
        totalRayMarchLength = hit2.t - hit1.t;
    } else if (cameraHeight < getCloudLayerMax()) {
        // DEBUG
        if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
            return vec4(1.0, 1.0, 0.0, 0.0);
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
    // raymarch length is too long near horizon
    totalRayMarchLength = min(4.0 * (getCloudLayerMax() - getCloudLayerMin()), totalRayMarchLength);

    // DEBUG: intersection test failed
    if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
        return vec4(1.0, 0.0, 0.0, 1.0);
    }

    vec3 result = vec3(0.0); // final luminance
    bool isGround = false;

    bool useDebugColor = false;

    int numSteps = 64;
    float lengthRatio = totalRayMarchLength / (getCloudLayerMax() - getCloudLayerMin());
    lengthRatio = clamp(lengthRatio, 1.0, 2.0);
    numSteps = int(float(numSteps) * lengthRatio);

    const int inscatSteps = 6;
    
    float cosTheta = dot(camera.direction, -sunDir);
    float phaseFn = phaseHG(cosTheta, 0.42);

    float opticalThickness = 0.0;
    float T = 1.0; // transmittance
    
    vec3 L = vec3(0.0); // luminance
    
    vec3 P = raymarchStartPos; // Current sample position
    float seg = totalRayMarchLength / float(numSteps);
    vec3 P_step = camera.direction * seg; // Fixed step size between sample positions

    float occluderDepth = texture(sceneDepth, uv).r;
    vec3 occluderPos = getViewPositionFromSceneDepth(uv, occluderDepth);

    // (#todo: empty-space optimization)
    // Raymarching
    for (int i = 0; i < numSteps; ++i)
    {
        if (P.y < 0.0)
        {
            isGround = true;
            break;
        }

        vec3 VP = getViewPositionFromWorldPosition(P);
        if (VP.z < occluderPos.z) {
            break;
        }

#if DEBUG_MODE == DEBUG_MODE_WEATHER
        return vec4(vec3(sampleWeather(P).x), 0.0);
#endif

        float cloudDensity = sampleCloud(P, float(i) / 16);
        
        float sigma_a = 0.003; // absorption coeff
        float sigma_s = 0.1; // scattering coeff

        // Raymarch from current position to Sun
        float TL = 1.0; // transmittance(P->Sun) or light visibility
        {
            hit_t hitL = no_hit;
            ray_t ray2 = ray_t(P, -sunDir);
            intersect_sphere(ray2, cloudOuterSphere, hitL);

            float tau = 0.0; // optical thickness
            if (isInvalidHit(hitL) == false) {
                // This is too large and will sample densities too far from P
                //float segLight = hitL.t / float(inscatSteps);

                float segLight = seg * 0.1 * float(inscatSteps);
                vec3 PL_step = ray2.direction * segLight;
                vec3 PL = P;

                float coneRadius = 1.0;
                const float CONE_STEP = 1.0/6.0;

                for (int j = 0; j < inscatSteps; ++j) {
#if 0 // Cone sample
                    vec3 samplePos = P + coneRadius * noiseKernel[j] * float(j);
                    tau += (sigma_a + sigma_s) * sampleCloud(samplePos, 0) * segLight;

                    PL += PL_step;
                    coneRadius += CONE_STEP;
#else
                    tau += (sigma_a + sigma_s) * sampleCloud(PL, 0) * segLight;
                    PL += PL_step;
#endif
                }
            }
            TL = exp(-tau);
        }

        // Lighting equations for volume rendering
        // dL(p,w) = Li(p,w) - Lo(p,w) = emission + scattering_in - scattering_out - absorption
        // absorption : dLa = -sigma_a * Li(p,w) * ds
        // emission   : dLe = Le(p,w) * ds
        // scattering : dLout(p,w) = -sigma_s * Li(p,w) * ds
        //              dLin(p,w) = sigma_s * phase_fn(w,w') * incoming_radiance(p,w') * ds

        float dOT = (sigma_a + sigma_s) * cloudDensity * seg;
        opticalThickness += dOT;
        float dT = exp(-dOT);
        T *= dT;

        // Emission
        // #todo: Does cloud emits light?
        // http://www.sc.chula.ac.th/courseware/2309507/Lecture/remote10.htm
        // #todo: Multiply with cloudDensity?
        vec3 Lem = T * (0.05 * vec3(0.13, 0.13, 0.27));

        // In-scattering
        vec3 SUN_RADIANCE = 0.4 * vec3(0.9, 0.8, 1.1); // #todo
        vec3 Lsc = T * sigma_s * phaseFn * (SUN_RADIANCE * TL);

        // #todo: Multiply with cloudDensity?
        vec3 Lsample = Lem + Lsc;
        L += Lsample;

        // Beer-Powder (GPU Pro 7: Real-Time Volumetric Clouds)
        bool usePowder = false;
        //usePowder = true;
        if (usePowder) {
            float powder_sugar_effect = 1.0 - exp(-opticalThickness * 2.0);
            float beers_law = exp(-opticalThickness);
            float light_energy = 2.0 * beers_law * powder_sugar_effect;
            L = mix(vec3(1.0), L, light_energy);
        }

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
#if DEBUG_TRANSMITTANCE
        result = vec3(1.0) - T; // DEBUG: transmittance
#else
        result = L;
#endif
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

vec3 getPrevViewDirection(vec2 uv) {
	vec3 P = vec3(2.0 * uv - 1.0, 0.0);
    P.x *= uboPerFrame.screenResolution.x / uboPerFrame.screenResolution.y;
    P.z = -(1.0 / tan(uboPerFrame.zRange.z * 0.5));
	P = normalize(P);
    
    mat3 camera_transform = mat3(uboPerFrame.prevInverseViewTransform);
	vec3 ray_forward = camera_transform * P;
    
    return ray_forward;
}
vec2 viewDirectionToUV_prevFrame(vec3 dir) {
    vec3 P = mat3(uboPerFrame.prevViewTransform) * dir;

#if 1
    float zOnPlane = -(1.0 / tan(uboPerFrame.zRange.z * 0.5));
    P.xy *= zOnPlane / P.z;
    P.x *= uboPerFrame.screenResolution.y / uboPerFrame.screenResolution.x;
    return 0.5 * (P.xy + vec2(1.0));
#else
    float a = P.x;
    float b = P.y;
    float c = P.z;
    float z = -(1.0 / tan(uboPerFrame.zRange.z * 0.5));
    float k = uboPerFrame.screenResolution.y / uboPerFrame.screenResolution.x;
    float m = z * sqrt(1.0 - c*c) / (c * sqrt(a*a + b*b));
    float u = 0.5 * (1.0 + a*m*k);
    float v = 0.5 * (1.0 + b*m);
    return vec2(u, v);
#endif
}

void main() {
    ivec2 sceneSize = imageSize(renderTarget);
    if (gl_GlobalInvocationID.x >= sceneSize.x || gl_GlobalInvocationID.y >= sceneSize.y) {
		return;
	}

    ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(currentTexel) / vec2(sceneSize - ivec2(1,1)); // [0.0, 1.0]
    vec3 viewDir = getViewDirection(uv);
    vec3 prevViewDir = getPrevViewDirection(uv);

#if TEMPORAL_REPROJECTION

#define REPROJECTION_METHOD 1
// #todo: Bad at camera rotation or at the interface between cloud and non-cloud texels
// #todo: Neighborhood clamping
#if REPROJECTION_METHOD == 1
    const vec2 REPROJECTION_FETCH_OFFSET = vec2(0.5) / sceneSize;
    const float REPROJECTION_INVALID_ANGLE = -1.0;//cos(0.0174533); // 1 degrees
    uint bayerIndex = (gl_GlobalInvocationID.y % 4) * 4 + (gl_GlobalInvocationID.x % 4);
    if (bayerIndex != bayerPattern[uboCloud.frameCounter % 16]) {
        // #todo: Am I doing this wrong?
        vec2 prevUV = viewDirectionToUV_prevFrame(viewDir);
        if (0.0 <= prevUV.x && prevUV.x < 1.0 && 0.0 <= prevUV.y && prevUV.y < 1.0) {
            //ivec2 prevTexel = ivec2(prevUV * vec2(sceneSize) + REPROJECTION_FETCH_OFFSET);
            ivec2 prevTexel = ivec2(prevUV * vec2(sceneSize - ivec2(1,1)) + REPROJECTION_FETCH_OFFSET);
            vec4 prevResult = texelFetch(reprojectionHistory, prevTexel, 0);
            //vec4 prevResult = texture(reprojectionHistory, prevUV + REPROJECTION_FETCH_OFFSET, 0);
            imageStore(renderTarget, currentTexel, prevResult);
            return;
        }
    }
// Reference quality for static camera
#elif REPROJECTION_METHOD == 2
    uint bayerIndex = (gl_GlobalInvocationID.y % 4) * 4 + (gl_GlobalInvocationID.x % 4);
    if (bayerIndex != bayerPattern[uboCloud.frameCounter % 16]) {
        vec4 prevResult = texelFetch(reprojectionHistory, currentTexel, 0);
        imageStore(renderTarget, currentTexel, prevResult);
        return;
    }
#endif // REPROJECTION_METHOD
#endif // TEMPORAL_REPROJECTION

	ray_t eye_ray;
    eye_ray.origin = uboPerFrame.ws_eyePosition;
	eye_ray.direction = viewDir;

    // Raymarching is broken if eye is too close to the interface of cloud layers
    if (abs(eye_ray.origin.y - getCloudLayerMin()) <= 1.5 || abs(eye_ray.origin.y - getCloudLayerMax()) <= 1.5) {
        eye_ray.origin.y += 1.5;
    }
    
	vec3 sunDir = uboPerFrame.directionalLights[0].wsDirection;

    vec4 result = scene(eye_ray, sunDir, uv);

    imageStore(renderTarget, currentTexel, result);
}
