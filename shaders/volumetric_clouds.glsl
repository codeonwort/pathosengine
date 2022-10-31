#version 460 core

#include "common.glsl"
#include "geom_common.glsl"
#include "deferred_common.glsl"

// ------------------------------------------------------------
// References
// [SIG2017] SIGGRAPH 2017, "Nubis: Authoring Real-Time Volumetric Cloudscapes with the Decima Engine"
//           https://advances.realtimerendering.com/s2017/index.html
// [GPUPRO7] GPU Pro 7, "Real-Time Volumetric Cloudscapes"

// ------------------------------------------------------------
// Tuning

// 0: noiseShape.tga and noiseErosion.tga
// 1: noiseShapePacked.tga and noiseErosionPacked.tga
#define PACKED_NOISE_TEXTURES        0

// #todo-cloud-wip: Temporal reprojection
#define TEMPORAL_REPROJECTION        0

// #todo-cloud-wip: No visual difference?
#define CONE_SAMPLING_ENABLED        1
#define CONE_SAMPLING_INIT_RADIUS    1.0
#define CONE_SAMPLING_STEP           (1.0 / 6.0)

// #todo-wip: Should look good with these values (reference: x10 times)
#define RAYMARCH_PRIMARY_MIN_STEP    54
#define RAYMARCH_PRIMARY_MAX_STEP    96
#define RAYMARCH_SECONDARY_STEP      6
#define RAYMARCH_MIN_TRANSMITTANCE   0.01

// Reduce step size if hits a density.
#define FINE_MARCH_FACTOR            0.25
// Restore coarse step size if fine march hits nothing several times.
#define FINE_MARCH_EXIT_COUNT        10

#define CLOUD_EMIT_LIGHT             0
#define CLOUD_ABSOPRTION_COEFF       0.9
// For both in-scattering and out-scattering
#define CLOUD_SCATTER_COEFF          0.5
// Absoprtion + out-scattering
#define CLOUD_EXTINCTION_COEFF       min(1.0, CLOUD_ABSOPRTION_COEFF + CLOUD_SCATTER_COEFF)

// ------------------------------------------------------------
// Debugging

#define DEBUG_MODE            0 // Set to one of values for visualization
#define DEBUG_MODE_WEATHER    1 // Cloud coverage in weather texture
#define DEBUG_MODE_NO_NOISE   2 // Raymarching result without applying cloud noise
#define DEBUG_MODE_NO_EROSION 3 // Apply shape noise, but no erosion noise

#define DEBUG_TRANSMITTANCE   0
#define DEBUG_MINUS_COLOR     0

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
vec2 getWindSpeed() { return vec2(uboCloud.windSpeedX, uboCloud.windSpeedZ); }
float getWeatherScale() { return uboCloud.weatherScale; }
float getBaseNoiseScale() { return uboCloud.baseNoiseScale; }
float getErosionNoiseScale() { return uboCloud.erosionNoiseScale; }
float getCloudCurliness() { return uboCloud.cloudCurliness; }
vec3 getSunIntensity() { return uboCloud.sunIntensity.xyz; }
vec3 getSunDirection() { return uboCloud.sunDirection.xyz; }

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
#define STRATUS_GRADIENT vec4(0.0, 0.1, 0.2, 0.3)
#define STRATOCUMULUS_GRADIENT vec4(0.02, 0.2, 0.48, 0.625)
#define CUMULUS_GRADIENT vec4(0.00, 0.1625, 0.88, 0.98)
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
	wPos.xz += getWorldTime() * getCloudLayerMin() * getWindSpeed() / getWeatherScale();
	vec2 uv = vec2(0.5) + (getWeatherScale() * wPos.xz / getCloudLayerMin());
	// (lowCoverage, highCoverage, cloudType, ?)
	vec4 rawData = textureLod(inWeatherMap, uv, 0);

	WeatherData weather;
	weather.cloudCoverage = max(rawData.x, saturate(rawData.y * uboCloud.globalCoverage));
	weather.cloudType = rawData.z;

	return weather;
}

// #todo: Replace with a packed texture
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
	return saturate(baseCloudWithCoverage);
#endif

	// 3. Apply erosion
	float erosionModifier = saturate(mix(erosion, 1.0 - erosion, saturate(heightFraction * 10.0)));
	float finalCloud = baseCloudWithCoverage - erosionModifier * baseCloudWithCoverage;
	finalCloud = saturate(remap(finalCloud, erosionModifier * 0.2, 1.0, 0.0, 1.0));

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

	// Additionally jitter sample position with STBN
	// #todo: Determine jitter multiplier? 25.0 is my empirical value.
	raymarchStartPos += 25.0 * camera.direction * (stbn - 0.5);

	// Raymarch length is too long near horizon.
	totalRayMarchLength = min(16.0 * (getCloudLayerMax() - getCloudLayerMin()), totalRayMarchLength);

	// DEBUG: intersection test failed
	if (isInvalidHit(hit1) && isInvalidHit(hit2)) {
		return vec4(1.0, 0.0, 0.0, 1.0);
	}

	vec3 result = vec3(0.0); // Final luminance or debug output
	bool isGround = false;

	bool useDebugColor = false;

	float primaryLengthRatio = totalRayMarchLength / (getCloudLayerMax() - getCloudLayerMin());
	primaryLengthRatio = clamp(primaryLengthRatio, 1.0, 2.0) - 1.0;
	const int numPrimarySteps = int(mix(
		float(RAYMARCH_PRIMARY_MIN_STEP),
		float(RAYMARCH_PRIMARY_MAX_STEP),
		primaryLengthRatio));
	const int numLightingSteps = RAYMARCH_SECONDARY_STEP;
	
	float cosTheta = dot(camera.direction, -sunDir);
	float phaseFn = phaseHG(cosTheta, 0.42);

	float opticalThickness = 0.0;
	float T = 1.0;      // Transmittance
	vec3 L = vec3(0.0); // Luminance
	
	vec3 currentPos = raymarchStartPos; // Current sample position
	float primaryStepLength = totalRayMarchLength / float(numPrimarySteps);
	vec3 primaryStep = camera.direction * primaryStepLength; // Fixed step size between sample positions
	bool bCoarseMarch = true;
	int fineMarchMissCount = 0;

	float occluderDepth = texture(inSceneDepth, uv).r;
	vec3 occluderPosVS = getViewPositionFromSceneDepth(uv, occluderDepth);

	// #todo: empty-space optimization
	// Raymarching
	for (int i = 0; i < numPrimarySteps; ++i) {
		if (currentPos.y < 0.0) {
			isGround = true;
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

		if (bCoarseMarch) {
			if (sampleCloudCoarse(currentPos, cloudLOD) > 0.0) {
				currentPos -= primaryStep;
				primaryStepLength *= FINE_MARCH_FACTOR;
				primaryStep *= FINE_MARCH_FACTOR;
				bCoarseMarch = false;
				currentPos += primaryStep;
			}
		}

		float cloudDensity = sampleCloud(currentPos, cloudLOD);

		if (!bCoarseMarch) {
			if (cloudDensity > 0.0) {
				fineMarchMissCount = 0;
			} else {
				fineMarchMissCount += 1;
				if (fineMarchMissCount > FINE_MARCH_EXIT_COUNT) {
					primaryStepLength /= FINE_MARCH_FACTOR;
					primaryStep /= FINE_MARCH_FACTOR;
					bCoarseMarch = true;
					fineMarchMissCount = 0;
				}
			}
		}

		// #todo: Separate this logic to sampleLight()?
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

				float lightStepLength = 0.2 * primaryStepLength / float(numLightingSteps);
				vec3 lightStep = lightRay.direction * lightStepLength;
				vec3 lightSamplePos = currentPos;

				float coneRadius = CONE_SAMPLING_INIT_RADIUS;
				for (int j = 0; j < numLightingSteps; ++j) {
#if CONE_SAMPLING_ENABLED
					vec3 samplePos = lightSamplePos + coneRadius * noiseKernel[j] * float(j);
					tau += CLOUD_SCATTER_COEFF * sampleCloud(samplePos, 0) * lightStepLength;
					lightSamplePos += lightStep;
					coneRadius += CONE_SAMPLING_STEP;
#else
					tau += CLOUD_SCATTER_COEFF * sampleCloud(lightSamplePos, 0) * lightStepLength;
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
			T *= dT;
		} // if (cloudDensity > 0.0)
		
		// Emission (#todo: Does cloud emits light?)
		vec3 Lem = vec3(0.0);
#if CLOUD_EMIT_LIGHT
		// http://www.sc.chula.ac.th/courseware/2309507/Lecture/remote10.htm
		if (cloudDensity > 0.0) {
			Lem = 0.05 * vec3(0.13, 0.13, 0.27);
		}
#endif

		// In-scattering
#if 1
		// #todo-wip: Inscattered light should be added only if cloud is there, but then too dark.
		vec3 Lsc = vec3(0.0);
		if (cloudDensity > 0.0) {
			Lsc += CLOUD_SCATTER_COEFF * phaseFn * (getSunIntensity() * TL);
		}
#else
		vec3 Lsc = CLOUD_SCATTER_COEFF * phaseFn * (getSunIntensity() * TL);
#endif

		if (cloudDensity > 0.0) {
			float heightFraction = getHeightFraction(currentPos);
			float depthProb = 0.05 + pow(cloudDensity, remap(heightFraction, 0.0, 0.85, 0.5, 2.0));
			float verticalProb = pow(remap(heightFraction, 0.07, 0.14, 0.1, 1.0), 0.8);
			float inscatterProb = depthProb * verticalProb;
			Lsc *= inscatterProb;
		}

		vec3 Lsample = Lem + Lsc;
		L += T * Lsample;

		if (T < RAYMARCH_MIN_TRANSMITTANCE) {
			break;
		}

#if DEBUG_MINUS_COLOR
		if (isMinusColor(Lsc)) {
			useDebugColor = true;
			result = vec3(1.0, 0.0, 0.0);
			break;
		}
#endif

		currentPos += primaryStep;
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

#if DEBUG_MINUS_COLOR
	if (isMinusColor(result) && useDebugColor == false) {
		result = vec3(0.0, 1.0, 1.0);
	}
#endif

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
	ivec2 sceneSize = imageSize(outRenderTarget);
	ivec2 currentTexel = ivec2(gl_GlobalInvocationID.xy);

	if (any(greaterThanEqual(currentTexel, sceneSize))) {
		return;
	}
	vec2 uv = vec2(currentTexel) / vec2(sceneSize - ivec2(1,1)); // [0.0, 1.0]
	vec3 viewDir = getViewDirection(uv);
	vec3 prevViewDir = getPrevViewDirection(uv);

	// NOTE: GL_REPEAT has no effect to texelFetch()
	ivec3 stbnPos = ivec3(currentTexel.x % 128, currentTexel.y % 128, uboCloud.frameCounter % 64);
	float stbn = texelFetch(inSTBN, stbnPos, 0).x;

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
			vec4 prevResult = texelFetch(inReprojectionHistory, prevTexel, 0);
			//vec4 prevResult = texture(inReprojectionHistory, prevUV + REPROJECTION_FETCH_OFFSET, 0);
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

	// Raymarching will be broken if eye location is too close to the interface of cloud layers
	if (abs(cameraRay.origin.y - getCloudLayerMin()) <= 1.5 || abs(cameraRay.origin.y - getCloudLayerMax()) <= 1.5) {
		cameraRay.origin.y += 1.5;
	}
	
	vec3 sunDir = getSunDirection();

	// (x, y, z) = luminance, w = transmittance
	vec4 outResult = traceScene(cameraRay, sunDir, uv, stbn);
	imageStore(outRenderTarget, currentTexel, outResult);
}
