#version 460 core

#include "deferred_common.glsl"
#include "shadow_mapping.glsl"
#include "brdf.glsl"

#define LIGHT_SOURCE_DIRECTIONAL 0
#define LIGHT_SOURCE_POINT       1
#define LIGHT_SOURCE_RECT        2
// Just suppress syntax error.
#ifndef LIGHT_SOURCE_TYPE
	#define LIGHT_SOURCE_TYPE LIGHT_SOURCE_DIRECTIONAL
#endif

#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_DIRECTIONAL
	#define LIGHT_STRUCT DirectionalLight
#elif LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT
	#define LIGHT_STRUCT PointLight
#elif LIGHT_SOURCE_TYPE == LIGHT_SOURCE_RECT
	#define LIGHT_STRUCT RectLight
#endif

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_DirectLighting {
	uint enableShadowing;
	uint haveShadowMap;
	int omniShadowMapIndex;
	uint _padding0;

	LIGHT_STRUCT lightParameters;
} ubo;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform usampler2D gbuf2;
// reserved for more gbuffers...
layout (binding = 5) uniform sampler2D ssaoMap;
layout (binding = 6) uniform sampler2DArrayShadow csm;
layout (binding = 7) uniform samplerCubeArrayShadow omniShadowMaps;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

// Getters for UBO
bool isShadowingEnabled()         { return ubo.enableShadowing != 0; }
bool haveShadowMap()              { return ubo.haveShadowMap != 0; }
LIGHT_STRUCT getLightParameters() { return ubo.lightParameters; }

float getShadowing(GBufferData gbufferData) {
	ShadowQuery query;
	query.vPos    = gbufferData.vs_coords;
	query.wPos    = gbufferData.ws_coords;
	query.vNormal = gbufferData.normal;
	query.wNormal = gbufferData.ws_normal;
	
	return getShadowingFactor(csm, query);
}

float getShadowingByPointLight(GBufferData gbufferData, PointLight light, int shadowMapIndex) {
	OmniShadowQuery query;
	query.shadowMapIndex    = shadowMapIndex;
	query.lightPos          = light.worldPosition;
	query.attenuationRadius = light.attenuationRadius;
	query.wPos              = gbufferData.ws_coords;

	return getOmniShadowingFactor(omniShadowMaps, query);
}

// Wo in view space
vec3 getOutgoingDirection(GBufferData gbufferData) {
	return normalize(uboPerFrame.eyePosition - gbufferData.vs_coords);
}

float getSphereLightRoughness(PointLight light, float distance, float roughness) {
	return clamp(roughness + light.sourceRadius / (3.0 * distance), 0.0, 1.0);
}

// Point light to sphere light.
// Find representative point.
void toSphereLight(
	PointLight light, GBufferData gbufferData,
	out vec3 outNewL, out vec3 outNewPos)
{
	vec3 L = light.positionVS - gbufferData.vs_coords;
	vec3 N = gbufferData.normal;
	vec3 R = reflect(-getOutgoingDirection(gbufferData), N);
	vec3 centerToRay = -(L - dot(L, R) * R);
	vec3 surfaceToRep = L + centerToRay * clamp(light.sourceRadius / length(centerToRay), 0.0, 1.0);

	outNewL = normalize(surfaceToRep);
	outNewPos = gbufferData.vs_coords + surfaceToRep;
}

// SIGGRAPH 2014: "Moving Frostbite to PBR"
float rightPyramidSolidAngle(float dist, float halfWidth, float halfHeight) {
	float a = halfWidth;
	float b = halfHeight;
	float h = dist;

	return 4.0 * asin(a * b / sqrt((a * a + h * h) * (b * b + h * h)));
}

// #todo-light: Redundant logic. Hopefully shader compiler will optimize it out...
// Wi is in view space.
// WARNING: Negate of actual incoming direction
//          following W_i convention in The Rendering Equation.
vec3 getIncomingDirection(GBufferData gbufferData) {
	LIGHT_STRUCT light = getLightParameters();

#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_DIRECTIONAL
	return -light.vsDirection;

#elif LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT
	vec3 L, unused;
	toSphereLight(light, gbufferData, L, unused);
	return L;
	//return normalize(light.positionVS - gbufferData.vs_coords);

#elif LIGHT_SOURCE_TYPE == LIGHT_SOURCE_RECT
	return -light.directionVS;

#else
	#error "Invalid light source type"
#endif
}

vec3 getIncomingRadiance(GBufferData gbufferData, vec3 V) {
	vec3 radiance = vec3(0.0);
	bool bEnableShadowMap = isShadowingEnabled() && haveShadowMap();

#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_DIRECTIONAL
	DirectionalLight light = getLightParameters();

	vec3 L = -light.vsDirection;
	radiance = light.intensity;

	if (bEnableShadowMap) {
		radiance *= getShadowing(gbufferData);
	}
#endif // LIGHT_SOURCE_TYPE == LIGHT_SOURCE_DIRECTIONAL

#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT
	PointLight light = getLightParameters();

	//vec3 L = normalize(light.positionVS - gbufferData.vs_coords);
	vec3 L, lightPos;
	toSphereLight(light, gbufferData, L, lightPos);

	float distance = length(lightPos - gbufferData.vs_coords);

	if (distance > light.attenuationRadius) {
		discard;
	}

	radiance = light.intensity;
	radiance *= pointLightFalloff(light.attenuationRadius, distance);

	// Sphere normalization
	float newRough = getSphereLightRoughness(light, distance, gbufferData.roughness);
	float sphereNorm = gbufferData.roughness / newRough;
	sphereNorm = sphereNorm * sphereNorm;
	radiance *= sphereNorm;

	if (bEnableShadowMap) {
		radiance *= getShadowingByPointLight(gbufferData, light, ubo.omniShadowMapIndex);
	}
#endif // LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT

#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_RECT
	RectLight light = getLightParameters();

	vec3 Wi = -light.directionVS;

	// Check if the light reaches at the surface point.
	vec3 v = gbufferData.vs_coords - light.virtualCenterVS;
	float distComp = dot(v, -Wi) / light.virtualOffset;
	vec3 up = light.upVS;
	vec3 right = light.rightVS;
	bool bDirectHit =
		abs(dot(v, right)) <= 0.5 * light.halfWidth * distComp
		&& abs(dot(v, up)) <= 0.5 * light.halfHeight * distComp;

	// #todo-light: Adjust specular term for rect light
	if (bDirectHit) {
		float distance = length(light.positionVS - gbufferData.vs_coords);
		float distFalloff = pointLightFalloff(light.attenuationRadius, distance);
		float angularFalloff = rightPyramidSolidAngle(distance, light.halfWidth, light.halfHeight);
		radiance = light.intensity * distFalloff * angularFalloff;
	}
#endif // LIGHT_SOURCE_TYPE == LIGHT_SOURCE_RECT

	return radiance;
}

// Diffuse  : Perfect lambertian
// Specular : Generalized microfacet
vec3 CookTorranceBRDF(GBufferData gbufferData) {
	// NOTE: All vectors are in view space.
	vec3 N = gbufferData.normal;                // Surface normal
	vec3 V = getOutgoingDirection(gbufferData); // Wo
	vec3 L = getIncomingDirection(gbufferData); // Wi
	vec3 H = normalize(V + L);                  // Half vector

	float NdotL = max(dot(N, L), 0.0);
	float NdotV = max(dot(N, V), 0.0);

	vec3 albedo = gbufferData.albedo;
	float metallic = gbufferData.metallic;
	float roughness = gbufferData.roughness;
	float localAO = gbufferData.ao;

#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT
	// SIGGRAPH 2013: Real Shading in Unreal Engine 4 by Brian Karis, Epic Games (course note p.14)
	{
		LIGHT_STRUCT light = getLightParameters();
		vec3 lightPos;
		toSphereLight(light, gbufferData, L, lightPos);
		float distL = length(lightPos - gbufferData.vs_coords);
		roughness = getSphereLightRoughness(light, distL, roughness);
	}
	// L is renewed. Update all affected variables.
	H = normalize(V + L);
	NdotL = max(dot(N, L), 0.0);
#endif

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(albedo, vec3(1.0)), metallic);

	// Micro
	float D = distributionGGX(N, H, roughness);
	float G = geometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	vec3 diffuse = albedo * (1.0 - metallic);
	vec3 specular = (D * F * G) / max(4.0 * NdotV * NdotL, 0.001);

	vec3 Li = getIncomingRadiance(gbufferData, V);
	vec3 Lo = (kD * diffuse / PI + specular) * Li * NdotL;
	
	// #todo-light: SSAO is being sampled per each light unnecessarily
	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;
	vec3 finalRadiance = Lo * localAO * ssao;

	return finalRadiance;
}

vec3 getLocalIllumination(GBufferData gbufferData) {
	uint ID = gbufferData.material_id;

	vec3 result = vec3(0.0, 0.0, 0.0);
	if (ID == MATERIAL_ID_WIREFRAME || ID == MATERIAL_ID_ALPHAONLY) {
		result = gbufferData.albedo;
	} else if (ID == MATERIAL_ID_SOLID_COLOR || ID == MATERIAL_ID_PBR || ID == MATERIAL_ID_TEXTURE) {
		result = CookTorranceBRDF(gbufferData);
	} else {
		discard;
	}
	return result;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbuf0, gbuf1, gbuf2, gbufferData);

	vec3 radiance = getLocalIllumination(gbufferData);

// shadow_mapping.glsl
#if DEBUG_CSM_ID
	radiance = vec3(getShadowing(gbufferData));
#endif

	// #todo-light: Sometimes radiance is NaN.
	radiance = max(vec3(0.0), radiance);

	outSceneColor.rgb = radiance;
	outSceneColor.a = 0.0;
}
