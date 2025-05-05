#version 460 core

#include "../core/common.glsl"
#include "../core/brdf.glsl"
#include "../core/image_based_lighting.glsl"
#include "../core/diffuse_sh.glsl"
#include "../deferred_common.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} interpolants;

layout (std140, binding = 1) uniform UBO_IndirectSpecularLighting {
	uint  lightingMode; // cvar_indirectLighting
	uint  numReflectionProbes;
	uint  _pad0;
	uint  _pad1;

	float skyLightBoost;
	float specularBoost;
	float skyReflectionProbeMaxLOD;
	float localReflectionProbeMaxLOD;
} ubo;

layout (std140, binding = 2) readonly buffer SSBO_ReflectionProbe {
	ReflectionProbe reflectionProbeInfo[];
} ssbo;

layout (binding = 0) uniform usampler2D       gbufferA;
layout (binding = 1) uniform sampler2D        gbufferB;
layout (binding = 2) uniform usampler2D       gbufferC;
layout (binding = 3) uniform sampler2D        ssaoMap;
layout (binding = 4) uniform samplerCube      skyReflectionProbe;     // Sky specular IBL
layout (binding = 5) uniform sampler2D        brdfIntegrationMap;     // Precomputed table for specular IBL
layout (binding = 6) uniform samplerCubeArray localRadianceCubeArray; // Prefiltered local specular IBLs

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

vec3 getIndirectSpecular(GBufferData gbufferData) {
	const vec3 surfacePositionWS   = gbufferData.ws_coords;
	const vec3 surfaceNormalWS     = gbufferData.ws_normal;
	const float roughness          = gbufferData.roughness;
	const vec3 viewDirectionWS     = normalize(uboPerFrame.cameraPositionWS - surfacePositionWS);
	const vec3 specularDirectionWS = reflect(-viewDirectionWS, surfaceNormalWS);

	// Find the closest reflection probe.
	int probeIndex = -1;
	float probeMinDist = 60000.0f;
	for (uint i = 0; i < ubo.numReflectionProbes; ++i) {
		ReflectionProbe probe = ssbo.reflectionProbeInfo[i];
		float dist = length(surfacePositionWS - probe.positionWS);
		// #todo: Fetch probe depth and check if this reflection probe is visible from the surface point.
		if (dist <= probe.captureRadius && dist < probeMinDist) {
			dist = probeMinDist;
			probeIndex = int(probe.cubemapIndex);
		}
	}

	// Sample the reflection probe.
	vec3 specularSample;
	if (probeIndex == -1) {
		specularSample = textureLod(skyReflectionProbe, specularDirectionWS, roughness * ubo.skyReflectionProbeMaxLOD).rgb;
		specularSample *= ubo.skyLightBoost;
	} else {
		vec4 R4 = vec4(specularDirectionWS, float(probeIndex));
		specularSample = textureLod(localRadianceCubeArray, R4, roughness * ubo.localReflectionProbeMaxLOD).rgb;
		specularSample *= ubo.specularBoost;
		//if (probeIndex == 0) specularSample = vec3(10.0, 10.0, 0.0);
		//if (probeIndex == 1) specularSample = vec3(0.0, 10.0, 0.0);
		//if (probeIndex == 2) specularSample = vec3(0.0, 0.0, 10.0);
		//if (probeIndex == 3) specularSample = vec3(10.0, 0.0, 10.0);
	}

	return specularSample;
}

vec3 getGlobalIllumination_defaultLit(GBufferData gbufferData) {
	const vec3 surfacePositionWS   = gbufferData.ws_coords;
	const vec3 surfaceNormalWS     = gbufferData.ws_normal;
	const vec3 viewDirectionWS     = normalize(uboPerFrame.cameraPositionWS - surfacePositionWS);
	const float NdotV              = max(0.0, dot(surfaceNormalWS, viewDirectionWS));

	const vec3 albedo              = gbufferData.albedo;
	const float metallic           = gbufferData.metallic;
	const float roughness          = gbufferData.roughness;

	const vec3 F0                  = mix(vec3(0.04), min(albedo, vec3(1.0)), metallic);
	const vec3 F                   = fresnelSchlickRoughness(NdotV, F0, roughness);
	const vec3 kS                  = F;
	const vec3 kD                  = (1.0 - metallic) * (1.0 - kS);
	const vec2 envBRDF             = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;

	vec3 indirectSpecular = getIndirectSpecular(gbufferData);
	float ambientOcclusion = texture2D(ssaoMap, interpolants.screenUV).r;

	vec3 finalLighting = (kS * envBRDF.x + envBRDF.y) * indirectSpecular;
	// #todo-gi: Specular occlusion for indirect specular
	finalLighting *= ambientOcclusion;

	return finalLighting;
}

vec3 getGlobalIllumination(GBufferData gbufferData) {
	const uint shadingModel = gbufferData.material_id;
	vec3 lighting = vec3(0.0);

	if (shadingModel == MATERIAL_SHADINGMODEL_DEFAULTLIT) {
		lighting.rgb = getGlobalIllumination_defaultLit(gbufferData);
	}

	return lighting;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbufferA, gbufferB, gbufferC, gbufferData);

	vec3 lighting = getGlobalIllumination(gbufferData);
	lighting = max(vec3(0.0), lighting);

	// Additive blending
	outSceneColor = vec4(lighting, 0.0);
}
