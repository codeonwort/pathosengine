#version 460 core

#include "deferred_common.glsl"
#include "shadow_mapping.glsl"
#include "brdf.glsl"

#define LIGHT_SOURCE_DIRECTIONAL 0
#define LIGHT_SOURCE_POINT       1
// Just suppress syntax error.
#ifndef LIGHT_SOURCE_TYPE
	#define LIGHT_SOURCE_TYPE LIGHT_SOURCE_DIRECTIONAL
#endif

#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_DIRECTIONAL
	#define LIGHT_STRUCT DirectionalLight
#elif LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT
	#define LIGHT_STRUCT PointLight
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

// #todo-light: Redundant logic
vec3 getIncomingDirection(GBufferData gbufferData) {
	LIGHT_STRUCT light = getLightParameters();
#if LIGHT_SOURCE_TYPE == LIGHT_SOURCE_DIRECTIONAL
	return -light.vsDirection;
#elif LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT
	return normalize(light.viewPosition - gbufferData.vs_coords);
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

	vec3 L = normalize(light.viewPosition - gbufferData.vs_coords);
	float distance = length(light.viewPosition - gbufferData.vs_coords);
	float attenuation = pointLightAttenuation(light, distance);

	radiance = light.intensity;
	radiance *= attenuation;

	if (bEnableShadowMap) {
		radiance *= getShadowingByPointLight(gbufferData, light, ubo.omniShadowMapIndex);
	}
#endif // LIGHT_SOURCE_TYPE == LIGHT_SOURCE_POINT

	return radiance;
}

// Diffuse  : Perfect lambertian
// Specular : Cook-Torrance microfacet
vec3 CookTorranceBRDF(GBufferData gbufferData) {
	vec3 N = gbufferData.normal;
	vec3 V = normalize(uboPerFrame.eyePosition - gbufferData.vs_coords);

	vec3 N_world = gbufferData.ws_normal;
	vec3 V_world = normalize(uboPerFrame.ws_eyePosition - gbufferData.ws_coords);
	//vec3 R = reflect(-V_world, N_world);
	vec3 L = getIncomingDirection(gbufferData);
	vec3 H = normalize(V + L);

	vec3 albedo = gbufferData.albedo;
	float metallic = gbufferData.metallic;
	float roughness = gbufferData.roughness;
	float ao = gbufferData.ao;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(albedo, vec3(1.0)), metallic);

	float NDF = distributionGGX(N, H, roughness);
	float G = geometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 num = NDF * G * F;
	float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = num / max(denom, 0.001);

	float NdotL = max(dot(N, L), 0.0);
	vec3 Li = getIncomingRadiance(gbufferData, V);
	vec3 Lo = (kD * albedo / PI + specular) * Li * NdotL;

	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 finalRadiance = Lo * ssao;
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

	// #todo: Sometimes radiance.rgb is NaN.
	radiance = max(vec3(0.0), radiance);

	// output: standard shading
	outSceneColor.rgb = radiance;
	outSceneColor.a = 0.0;
}
