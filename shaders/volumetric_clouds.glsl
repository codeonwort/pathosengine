#version 460 core

#include "common.glsl"
#include "geom_common.glsl"
#include "deferred_common.glsl"

// ------------------------------------------------------------
// References
// [SIG2017] SIGGRAPH 2017, "Nubis: Authoring Real-Time Volumetric Cloudscapes with the Decima Engine"
//           https://advances.realtimerendering.com/s2017/index.html
// [GPUPRO7] GPU Pro 7, "Real-Time Volumetric Cloudscapes"
// [HANIKA] Fast Temporal Reprojection without Motion Vectors
//          https://jcgt.org/published/0010/03/02/

// ------------------------------------------------------------
// Tuning

// 0: noiseShape.tga and noiseErosion.tga
// 1: noiseShapePacked.tga and noiseErosionPacked.tga
#define PACKED_NOISE_TEXTURES        0

// #todo-cloud: Temporal reprojection
#define TEMPORAL_REPROJECTION        0

#define STBN_ENABLED                 1
#define STBN_SCALE                   50.0

#define RAYMARCH_PRIMARY_MIN_STEP    (54 * 1)
#define RAYMARCH_PRIMARY_MAX_STEP    (96 * 1)
#define RAYMARCH_SECONDARY_STEP      6
#define RAYMARCH_MIN_TRANSMITTANCE   0.05

#define CONE_SAMPLING_ENABLED        1
#define CONE_SAMPLING_INIT_RADIUS    10.0
#define CONE_SAMPLING_STEP           (CONE_SAMPLING_INIT_RADIUS / 6.0)

#define RAYMARCH_ADAPTIVE            1
// Reduce step size if hits a density.
#define FINE_MARCH_LENGTH            50.0
// Restore coarse step size if fine march hits nothing several times.
#define FINE_MARCH_EXIT_COUNT        10

// Clouds farther than this distance will be not seen.
#define DISTANCE_FALLOFF             100000.0

// #todo-cloud: Does cloud emits light?
#define CLOUD_EMIT_LIGHT             1

// If absorption or scattering coeff is too big (>= 0.1),
// cloud bottom layer will be too dark as sun light can't penetrate cloud density.
#define CLOUD_ABSOPRTION_COEFF       0.02
// For both in-scattering and out-scattering
#define CLOUD_SCATTER_COEFF          0.01
// Absoprtion + out-scattering
#define CLOUD_EXTINCTION_COEFF       min(1.0, CLOUD_ABSOPRTION_COEFF + CLOUD_SCATTER_COEFF)

// ------------------------------------------------------------
// Debugging

#define DEBUG_MODE            0 // Set to one of values for visualization
#define DEBUG_MODE_WEATHER    1 // Cloud coverage in weather texture
#define DEBUG_MODE_NO_NOISE   2 // Raymarching result without applying cloud noise
#define DEBUG_MODE_NO_EROSION 3 // Apply shape noise, but no erosion noise

// Replace luminance with transparency (which is 1.0 - transmittance)
#define DEBUG_TRANSPARENCY    0
// Check if volume lighting resulted in NaN or minus values.
#define DEBUG_BAD_LIGHTING    0

// ------------------------------------------------------------
// Input

layout (std140, binding = 1) uniform UBO_VolumetricCloud {
	float earthRadius;    // in meters
	float cloudLayerMinY; // in meters
	float cloudLayerMaxY; // in meters
	float windSpeedX;

	float windSpeedZ;
	float weatherScale;
	float baseNoiseScale;
	float erosionNoiseScale;

	vec4  sunIntensity; // (r, g, b, ?)
	vec4  sunDirection; // (x, y, z, ?)

	float cloudCurliness;
	float globalCoverage;
	float baseNoiseOffset;
	uint  frameCounter;
} uboCloud;

layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0) uniform sampler2D inSceneDepth;
layout (binding = 1) uniform sampler2D inWeatherMap;
layout (binding = 2) uniform sampler3D inShapeNoise;
layout (binding = 3) uniform sampler3D inErosionNoise;
layout (binding = 4) uniform sampler3D inSTBN;
layout (binding = 5) uniform sampler2D inReprojectionHistory; // cloud RT of prev frame
layout (binding = 6, rgba16f) writeonly uniform image2D outRenderTarget;

// ------------------------------------------------------------
// Constants

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

// ------------------------------------------------------------
// Utils

struct WeatherData {
	float cloudCoverage;
	float cloudType;
};

float getEarthRadius() { return uboCloud.earthRadius; }
float getCloudLayerMin() { return uboCloud.cloudLayerMinY; }
float getCloudLayerMax() { return uboCloud.cloudLayerMaxY; }
float getCloudLayerThickness() { return uboCloud.cloudLayerMaxY - uboCloud.cloudLayerMinY; }
vec2 getWindSpeed() { return vec2(uboCloud.windSpeedX, uboCloud.windSpeedZ); }
float getWeatherScale() { return uboCloud.weatherScale; }
float getBaseNoiseScale() { return uboCloud.baseNoiseScale; }
float getErosionNoiseScale() { return uboCloud.erosionNoiseScale; }
float getCloudCurliness() { return uboCloud.cloudCurliness; }
vec3 getSunIntensity() { return uboCloud.sunIntensity.xyz; }
vec3 getSunDirection() { return uboCloud.sunDirection.xyz; }

float getFOV() { return uboPerFrame.zRange.z; }
float getAspectRatioWH() { return uboPerFrame.zRange.w; }

float remap(float x, float oldMin, float oldMax, float newMin, float newMax) {
	return newMin + (newMax - newMin) * (x - oldMin) / (oldMax - oldMin);
}

float saturate(float x) {
	return clamp(x, 0.0, 1.0);
}

bool isMinusColor(vec3 v) {
	return v.x < 0.0 || v.y < 0.0 || v.z < 0.0;
}

// ------------------------------------------------------------
// Volumetric clouds

// Rayleigh
float phaseR(float cosTheta) {
	return 3.0 / (16.0 * PI) * (1.0 + cosTheta * cosTheta);
}
// Mie
float phaseM(float t) {
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

// bottom layer = 0.0, top layer = 1.0
float getHeightFraction(vec3 wPos) {
	float fraction = (wPos.y - getCloudLayerMin()) / (getCloudLayerMax() - getCloudLayerMin());
	return saturate(fraction);
}

// Density gradients over height for each cloud type
// cloudType = 0.0 -> stratus
// cloudType = 0.5 -> stratocumulus
// cloudType = 1.0 -> cumulus
// in-between values -> mix of these types
#define STRATUS_GRADIENT         vec4(0.00, 0.1000, 0.20, 0.300)
#define STRATOCUMULUS_GRADIENT   vec4(0.02, 0.2000, 0.48, 0.625)
#define CUMULUS_GRADIENT         vec4(0.00, 0.1625, 0.88, 0.980)
float getDensityOverHeight(float heightFraction, float cloudType) {
	float stratusFactor = 1.0 - clamp(cloudType * 2.0, 0.0, 1.0);
	float stratoCumulusFactor = 1.0 - abs(cloudType - 0.5) * 2.0;
	float cumulusFactor = clamp(cloudType - 0.5, 0.0, 0.5) * 2.0;
	vec4 baseGradient
		= stratusFactor * STRATUS_GRADIENT
		+ stratoCumulusFactor * STRATOCUMULUS_GRADIENT
		+ cumulusFactor * CUMULUS_GRADIENT;
	
	float x = saturate(remap(heightFraction, baseGradient.x, baseGradient.y, 0.0, 1.0));
	float y = saturate(remap(heightFraction, baseGradient.z, baseGradient.w, 1.0, 0.0));
	return x * y;
}

WeatherData sampleWeather(vec3 wPos) {
	vec2 uv = vec2(0.5) + (getWeatherScale() * wPos.xz / getCloudLayerMin());
	uv += getWorldTime() * getWindSpeed();
	// (lowCoverage, highCoverage, cloudType, ?)
	vec4 rawData = textureLod(inWeatherMap, uv, 0);

	WeatherData weather;
	weather.cloudCoverage = max(rawData.x, saturate(rawData.y * uboCloud.globalCoverage));
	weather.cloudType = rawData.z;

	return weather;
}

vec2 sampleCloudShapeAndErosion(vec3 wPos, float lod, float heightFraction) {
	vec3 samplePos = wPos;
	samplePos.xz += getWorldTime() * getWindSpeed() / getWeatherScale() * 128.0;
	samplePos.xz += heightFraction * getWindSpeed() / getWeatherScale() * 500.0;
	samplePos *= getBaseNoiseScale() / 128.0;

	vec3 samplePos2 = wPos;
	//samplePos2.xz += getWorldTime() * getWindSpeed() / getWeatherScale();
	samplePos2 *= getErosionNoiseScale() / 32.0;

	// R: Perlin-Worley noise
	// G,B,A: Worley noises at increasing frequencies
	vec4 baseNoises = textureLod(inShapeNoise, samplePos, lod);
#if PACKED_NOISE_TEXTURES
	float baseCloud = baseNoises.x;
#else
	float perlinWorley = baseNoises.x;
	float lowFreqFBM = dot(vec3(0.625, 0.25, 0.125), baseNoises.yzw);
	float baseCloud = saturate(remap(perlinWorley, -(1.0 - lowFreqFBM), 1.0, 0.0, 1.0));
#endif
	baseCloud = saturate(baseCloud + uboCloud.baseNoiseOffset);

	vec3 detailNoises = textureLod(inErosionNoise, getCloudCurliness() * samplePos2, lod).xyz;
#if PACKED_NOISE_TEXTURES
	float erosion = detailNoises.x;
#else
	float erosion = dot(vec3(0.625, 0.25, 0.125), detailNoises);
#endif

	return vec2(baseCloud, erosion);
}

// Returns final cloud density for raymarching, all factors considered.
// (cloud coverage, density gradient, detail noise, ...)
float sampleCloud(vec3 P, float lod) {
	WeatherData weather = sampleWeather(P);
	float cloudCoverage = weather.cloudCoverage;
	float cloudType = weather.cloudType;

	float heightFraction = getHeightFraction(P);

#if DEBUG_MODE == DEBUG_MODE_NO_NOISE || DEBUG_MODE == DEBUG_MODE_WEATHER
	return cloudCoverage * getDensityOverHeight(heightFraction, 1.0);
#endif

	vec2 cloudNoiseSamples = sampleCloudShapeAndErosion(P, lod, heightFraction);
	float baseCloud = cloudNoiseSamples.x;
	float erosion = cloudNoiseSamples.y;

	// 1. Apply density gradient
	baseCloud *= getDensityOverHeight(heightFraction, cloudType);
	// Reduce density at the bottoms of the clouds (density increases over altitude)
	baseCloud *= heightFraction;

	// 2. Apply cloudCoverage
	float baseCloudWithCoverage = saturate(remap(baseCloud, 1.0 - cloudCoverage, 1.0, 0.0, 1.0));

#if DEBUG_MODE == DEBUG_MODE_NO_EROSION
	return baseCloudWithCoverage;
#endif

	// 3. Apply erosion
	float erosionModifier = mix(erosion, 1.0 - erosion, saturate(heightFraction * 10.0));
	float finalCloud = remap(baseCloudWithCoverage, erosionModifier * 0.2, 1.0, 0.0, 1.0);
	finalCloud = saturate(finalCloud);

	return finalCloud;
}

// sampleCloud() without erosion noise.
float sampleCloudCoarse(vec3 P, float lod) {
	WeatherData weather = sampleWeather(P);
	float cloudCoverage = weather.cloudCoverage;
	float cloudType = weather.cloudType;

	float heightFraction = getHeightFraction(P);

	vec2 cloudNoiseSamples = sampleCloudShapeAndErosion(P, lod, heightFraction);
	float baseCloud = cloudNoiseSamples.x;

	// 1. Apply density gradient
	baseCloud *= getDensityOverHeight(heightFraction, cloudType);
	// Reduce density at the bottoms of the clouds (density increases over altitude)
	baseCloud *= heightFraction;

	// 2. Apply cloudCoverage
	float baseCloudWithCoverage = saturate(remap(baseCloud, 1.0 - cloudCoverage, 1.0, 0.0, 1.0));

	return baseCloudWithCoverage;
}

// Returns (xyz = luminance, w = transmittance)
vec4 traceScene(Ray camera, vec3 sunDir, vec2 uv, float stbn) {
	Sphere cloudInnerSphere = Sphere(vec3(0.0, -getEarthRadius(), 0.0), getEarthRadius() + getCloudLayerMin());
	Sphere cloudOuterSphere = Sphere(vec3(0.0, -getEarthRadius(), 0.0), getEarthRadius() + getCloudLayerMax());

	float cameraHeight = camera.origin.y;
	float totalRayMarchLength = 0.0;
	vec3 raymarchStartPos = vec3(0.0);

	HitResult hit1, hit2;
	hit1 = hit_ray_sphere(camera, cloudInnerSphere);
	hit2 = hit_ray_sphere(camera, cloudOuterSphere);
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

#if STBN_ENABLED
	// Additionally jitter the sample position with STBN.
	raymarchStartPos += STBN_SCALE * camera.direction * (stbn - 0.5);
#endif

	// Raymarch length is too long near horizon.
	totalRayMarchLength = min(16.0 * (getCloudLayerMax() - getCloudLayerMin()), totalRayMarchLength);

	// DEBUG: intersection test failed
	if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
		return vec4(1.0, 0.0, 0.0, 1.0);
	}

	// ------------------------------------------------------------
	// Prepare raymarching parameters

	// For distance falloff
	vec3 firstHitPos = camera.direction * totalRayMarchLength + raymarchStartPos;
	bool bFirstHit = true;

	float primaryLengthRatio = totalRayMarchLength / (getCloudLayerMax() - getCloudLayerMin());
	primaryLengthRatio = clamp(primaryLengthRatio, 1.0, 2.0) - 1.0;
	const int numPrimarySteps = int(mix(
		float(RAYMARCH_PRIMARY_MIN_STEP),
		float(RAYMARCH_PRIMARY_MAX_STEP),
		primaryLengthRatio));
	const int numLightingSteps = RAYMARCH_SECONDARY_STEP;
	
	vec3 currentPos = raymarchStartPos; // Current sample position
	float primaryStepLength = totalRayMarchLength / float(numPrimarySteps);
	vec3 primaryStep = camera.direction * primaryStepLength; // Fixed step size between sample positions

	bool bCoarseMarch = true;
	int fineMarchMissCount = 0;
	float primaryStepLengthBackup = primaryStepLength;
	vec3 primaryStepBackup = primaryStep;

	float occluderDepth = texture(inSceneDepth, uv).r;
	vec3 occluderPosVS = getViewPositionFromSceneDepth(uv, occluderDepth);

	// ------------------------------------------------------------
	// Prepare lighting parameters

	float finalTransmittance = 1.0;
	vec3 finalLuminance = vec3(0.0);

	bool bIsGround = false;

	float phaseFn = phaseHG(dot(camera.direction, -sunDir), 0.42);
	float opticalThickness = 0.0;

	// ------------------------------------------------------------
	// Perform raymarching

	for (int i = 0; i < numPrimarySteps; ++i) {
		if (currentPos.y < 0.0) {
			bIsGround = true;
			break;
		}

		// Stop raymarching if there is an occluder.
		vec3 currentPosVS = getViewPositionFromWorldPosition(currentPos);
		if (currentPosVS.z < occluderPosVS.z) {
			break;
		}

#if DEBUG_MODE == DEBUG_MODE_WEATHER
		return vec4(vec3(sampleWeather(currentPos).cloudCoverage), 0.0);
#endif

		float cloudLOD = 0.5 * float(i) / float(RAYMARCH_PRIMARY_MIN_STEP);

#if RAYMARCH_ADAPTIVE
		if (bCoarseMarch) {
			if (sampleCloudCoarse(currentPos, cloudLOD) > 0.0) {
				currentPos -= primaryStep;
				primaryStepLength = min(primaryStepLengthBackup, FINE_MARCH_LENGTH);
				primaryStep = camera.direction * primaryStepLength;
				bCoarseMarch = false;
				currentPos += primaryStep;
			}
		}
#endif // RAYMARCH_ADAPTIVE

		float cloudDensity = sampleCloud(currentPos, cloudLOD);

#if RAYMARCH_ADAPTIVE
		if (!bCoarseMarch) {
			if (cloudDensity > 0.0) {
				fineMarchMissCount = 0;
				if (bFirstHit) {
					bFirstHit = false;
					firstHitPos = currentPos;
				}
			} else {
				fineMarchMissCount += 1;
				if (fineMarchMissCount > FINE_MARCH_EXIT_COUNT) {
					primaryStepLength = primaryStepLengthBackup;
					primaryStep = primaryStepBackup;
					bCoarseMarch = true;
					fineMarchMissCount = 0;
				}
			}
		}
#endif // RAYMARCH_ADAPTIVE

		// Raymarch from current position to Sun.
		// - Attenuate transmittance and accumulate lighting.
		// - Only meaningful when cloudDensity is non-zero.
		float TL = 1.0; // transmittance(P->Sun) or 'light visibility'
		if (cloudDensity > 0.0) {
			Ray lightRay = Ray(currentPos, -sunDir);
			HitResult hitL = hit_ray_sphere(lightRay, cloudOuterSphere);

			float tau = 0.0; // Optical thickness
			if (isInvalidHit(hitL) == false) {
				// This is too large and will sample densities too far from currentPos
				//float lightStepLength = hitL.t / float(numLightingSteps);
				//float lightStepLength = 0.2 * primaryStepLength / float(numLightingSteps);
				float lightStepLength = 0.5 * getCloudLayerThickness() / float(numLightingSteps);

				vec3 lightStep = lightRay.direction * lightStepLength;
				vec3 lightSamplePos = currentPos;

				float coneRadius = CONE_SAMPLING_INIT_RADIUS;
				for (int j = 0; j < numLightingSteps; ++j) {
					float lightLOD = float(j) * 0.5;
#if CONE_SAMPLING_ENABLED
					vec3 samplePos = lightSamplePos + coneRadius * noiseKernel[j] * float(j);
					tau += CLOUD_SCATTER_COEFF * sampleCloud(samplePos, lightLOD) * lightStepLength;
					lightSamplePos += lightStep;
					coneRadius += CONE_SAMPLING_STEP;
#else
					tau += CLOUD_SCATTER_COEFF * sampleCloud(lightSamplePos, lightLOD) * lightStepLength;
					lightSamplePos += lightStep;
#endif
				}
			}
			TL = exp(-tau);

			// Lighting equations for volume rendering
			// dL(p,w) = Li(p,w) - Lo(p,w) = emission + scattering_in - scattering_out - absorption
			// absorption : dLa = -sigma_a * Li(p,w) * ds
			// emission   : dLe = Le(p,w) * ds
			// scattering : dLout(p,w) = -sigma_s * Li(p,w) * ds
			//              dLin(p,w) = sigma_s * phase_fn(w,w') * incoming_radiance(p,w') * ds

			float dOT = CLOUD_EXTINCTION_COEFF * cloudDensity * primaryStepLength;
			opticalThickness += dOT;
			float dT = exp(-dOT);
			finalTransmittance *= dT;
		} // if (cloudDensity > 0.0)
		
		vec3 Lem = vec3(0.0); // Emission
		vec3 Lsc = vec3(0.0); // In-scattering
		
		if (cloudDensity > 0.0) {
#if CLOUD_EMIT_LIGHT
			// http://www.sc.chula.ac.th/courseware/2309507/Lecture/remote10.htm
			Lem = 0.2 * cloudDensity * phaseFn * getSunIntensity() * vec3(0.13, 0.13, 0.27);
#endif
			Lsc = CLOUD_SCATTER_COEFF * phaseFn * (getSunIntensity() * TL);

			float heightFraction = getHeightFraction(currentPos);
			float depthProb = 0.05 + pow(cloudDensity, remap(heightFraction, 0.0, 0.85, 0.5, 2.0));
			float verticalProb = pow(remap(heightFraction, 0.07, 0.14, 0.1, 1.0), 0.8);
			float inscatterProb = depthProb * verticalProb;
			Lsc *= inscatterProb;
		} // if (cloudDensity > 0.0)

		vec3 Lsample = Lem + Lsc;
		finalLuminance += finalTransmittance * Lsample;

		if (finalTransmittance < RAYMARCH_MIN_TRANSMITTANCE) {
			break;
		}

		currentPos += primaryStep;
	}
	
	if (bIsGround) {
		finalLuminance = vec3(0.0, 0.0, 0.0);
	}

	// Distance falloff
	float firstHitDist = length(firstHitPos - camera.origin);
	// clipping ver.
	//if (firstHitDist >= DISTANCE_FALLOFF) { T = 1.0; }
	// attenuation ver.
	float distFalloff = pow(saturate(firstHitDist / DISTANCE_FALLOFF), 8.0);
	finalTransmittance = mix(finalTransmittance, 1.0, distFalloff);

#if DEBUG_BAD_LIGHTING
	if (any(isnan(finalLuminance)) || isMinusColor(finalLuminance)) {
		finalLuminance = vec3(1.0, 0.0, 0.0);
	}
#endif

#if DEBUG_TRANSPARENCY
	finalLuminance = vec3(1.0) - finalTransmittance;
#endif

	return vec4(finalLuminance, finalTransmittance);
}

vec3 getViewDirection(vec2 uv) {
	vec3 P = vec3(2.0 * uv - 1.0, 0.0);
	P.x *= getAspectRatioWH();
	P.z = -(1.0 / tan(getFOV() * 0.5));
	P = normalize(P);
	
	mat3 camera_transform = mat3(uboPerFrame.inverseViewTransform);
	vec3 ray_forward = camera_transform * P;
	
	return ray_forward;
}

// #todo-cloud: Implement proper temporal reprojection.
// Unlike TAA I can't derive exact prev/current world positions of first-hit cloud particles.
// This is never working well. Maybe I'll need to implement [HANIKA].
vec2 getPrevScreenUV(vec2 currentUV) {
	vec3 currentDir = getViewDirection(currentUV);
	vec3 P = mat3(uboPerFrame.prevViewTransform) * currentDir;

#if 1
	float zOnPlane = -(1.0 / tan(getFOV() * 0.5));
	P.xy *= zOnPlane / P.z;
	P.x /= getAspectRatioWH();
	return 0.5 * (P.xy + vec2(1.0));
#else
	// I don't remember where I got this from?
	float a = P.x;
	float b = P.y;
	float c = P.z;
	float z = -(1.0 / tan(getFOV() * 0.5));
	float k = 1.0 / getAspectRatioWH();
	float m = z * sqrt(1.0 - c*c) / (c * sqrt(a*a + b*b));
	float u = 0.5 * (1.0 + a*m*k);
	float v = 0.5 * (1.0 + b*m);
	return vec2(u, v);
#endif
}

void main() {
	ivec2 sceneSize = imageSize(outRenderTarget);
	ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);

	if (any(greaterThanEqual(currentTexel, sceneSize))) {
		return;
	}
	vec2 uv = vec2(currentTexel) / vec2(sceneSize - ivec2(1,1)); // [0.0, 1.0]
	vec3 viewDir = getViewDirection(uv);

	// NOTE: GL_REPEAT has no effect to texelFetch()
	ivec3 stbnPos = ivec3(currentTexel.x % 128, currentTexel.y % 128, uboCloud.frameCounter % 64);
	float stbn = texelFetch(inSTBN, stbnPos, 0).x;

#if TEMPORAL_REPROJECTION
#define REPROJECTION_METHOD 1
#if REPROJECTION_METHOD == 1
	vec2 REPROJECTION_FETCH_OFFSET = vec2(0.5) / sceneSize;
	const float REPROJECTION_INVALID_ANGLE = -1.0;//cos(0.0174533); // 1 degrees
	uint bayerIndex = (gl_GlobalInvocationID.y % 4) * 4 + (gl_GlobalInvocationID.x % 4);
	if (bayerIndex != bayerPattern[uboCloud.frameCounter % 16]) {
		vec2 prevUV = getPrevScreenUV(uv);
		if (0.0 <= prevUV.x && prevUV.x < 1.0 && 0.0 <= prevUV.y && prevUV.y < 1.0) {
			ivec2 prevTexel = ivec2(prevUV * vec2(sceneSize));
			vec4 prevResult = texelFetch(inReprojectionHistory, prevTexel, 0);
			imageStore(outRenderTarget, currentTexel, prevResult);
			return;
		}
	}
// Reference quality for static camera
#elif REPROJECTION_METHOD == 2
	uint bayerIndex = (gl_GlobalInvocationID.y % 4) * 4 + (gl_GlobalInvocationID.x % 4);
	if (bayerIndex != bayerPattern[uboCloud.frameCounter % 16]) {
		vec4 prevResult = texelFetch(inReprojectionHistory, currentTexel, 0);
		imageStore(outRenderTarget, currentTexel, prevResult);
		return;
	}
#endif // REPROJECTION_METHOD
#endif // TEMPORAL_REPROJECTION

	Ray cameraRay;
	cameraRay.origin = uboPerFrame.ws_eyePosition;
	cameraRay.direction = viewDir;

	// #todo-cloud: Raymarching will be broken if the camera is too close to the interfaces of cloud layers.
	// This constant is empirical and needs to be adaptive to cloud layer min/max heights.
	float layerD = 15.0;
	if (abs(cameraRay.origin.y - getCloudLayerMin()) <= layerD
		|| abs(cameraRay.origin.y - getCloudLayerMax()) <= layerD) {
		cameraRay.origin.y += layerD;
	}
	
	vec3 sunDir = getSunDirection();

	// (x, y, z) = luminance, w = transmittance
	vec4 outResult = traceScene(cameraRay, sunDir, uv, stbn);
	outResult.xyz = min(outResult.xyz, vec3(65535.0));

	imageStore(outRenderTarget, currentTexel, outResult);
}
