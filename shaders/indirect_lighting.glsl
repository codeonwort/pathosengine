#version 460 core

#include "common.glsl"
#include "deferred_common.glsl"
#include "brdf.glsl"

// #todo-light-probe: SH encoding for indirect diffuse

#define MAX_RADIANCE_PROBES 10
struct RadianceProbe {
	vec3 positionWS;
	float captureRadius;
};

struct IrradianceProbe {
	vec3 positionWS;
	float captureRadius;
	vec4 uvBounds;
};

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_IndirectLighting {
	float skyRadianceProbeMaxLOD; // Max LOD of sky probe
	float overallIntensity;
	uint numRadianceProbes;
	float radianceProbeMaxLOD; // Max LOD of local probes

	uvec4 irradianceAtlasParams; // (numActiveTiles, ?, ?, ?)

	RadianceProbe localRadianceProbes[MAX_RADIANCE_PROBES];
} ubo;

layout (std140, binding = 2) buffer SSBO_IndirectLighting {
	IrradianceProbe irradianceAtlasInfo[];
} ssbo;

layout (binding = 0) uniform usampler2D       gbufferA;
layout (binding = 1) uniform sampler2D        gbufferB;
layout (binding = 2) uniform usampler2D       gbufferC;
layout (binding = 3) uniform sampler2D        ssaoMap;
layout (binding = 4) uniform samplerCube      skyIrradianceProbe;     // Sky diffuse IBL
layout (binding = 5) uniform samplerCube      skyRadianceProbe;       // Sky specular IBL
layout (binding = 6) uniform sampler2D        brdfIntegrationMap;     // Precomputed table for specular IBL
layout (binding = 7) uniform samplerCubeArray localRadianceCubeArray; // Local specular IBLs
layout (binding = 8) uniform sampler2D        irradianceAtlas;        // Irradiance maps by local light probes

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

vec3 getImageBasedLighting(GBufferData gbufferData) {
	vec3 N = gbufferData.normal;
	vec3 V = normalize(uboPerFrame.eyePosition - gbufferData.vs_coords);
	float NdotV = max(dot(N, V), 0.0);

	vec3 N_world = gbufferData.ws_normal;
	vec3 V_world = normalize(uboPerFrame.ws_eyePosition - gbufferData.ws_coords);
	vec3 R_world = reflect(-V_world, N_world);

	vec3 albedo = gbufferData.albedo;
	float metallic = gbufferData.metallic;
	float roughness = gbufferData.roughness;
	float ao = gbufferData.ao;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(albedo, vec3(1.0)), metallic);
	
	vec3 Lo = vec3(0.0);

	vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	// Diffuse GI
	// Select up to 4 irradiance probes, blend them w.r.t. distance to the surface location.
	int localDiffuseIndex[4] = { -1, -1, -1, -1 };
	int numDiffuseIndex = 0;
	for (uint i = 0; i < ubo.irradianceAtlasParams.x; ++i) {
		IrradianceProbe probe = ssbo.irradianceAtlasInfo[i];
		vec3 dist3 = abs(gbufferData.ws_coords - probe.positionWS);
		float dist = length(gbufferData.ws_coords - probe.positionWS);

		bool bInRange = (dist <= probe.captureRadius);
		//bool bInRange = all(lessThanEqual(dist3, vec3(probe.captureRadius)));
		if (bInRange) {
			localDiffuseIndex[numDiffuseIndex] = int(i);
			++numDiffuseIndex;
			if (numDiffuseIndex == 4) {
				break;
			}
		}
	}
	vec3 diffuseIndirect = vec3(0.0);
	if (numDiffuseIndex == 0) {
		diffuseIndirect = texture(skyIrradianceProbe, N_world).rgb * albedo;
	} else {
		float distSum = 0.0;
		for (int i = 0; i < numDiffuseIndex; ++i) {
			distSum += length(gbufferData.ws_coords - ssbo.irradianceAtlasInfo[localDiffuseIndex[i]].positionWS);
		}

		vec2 uv = ONVEncode(N_world);
		for (int i = 0; i < numDiffuseIndex; ++i) {
			vec4 uvBounds = ssbo.irradianceAtlasInfo[localDiffuseIndex[i]].uvBounds;
			vec3 probePos = ssbo.irradianceAtlasInfo[localDiffuseIndex[i]].positionWS;
			vec2 atlasUV = uvBounds.xy + (uvBounds.zw - uvBounds.xy) * uv;

			vec3 diffuseSample = textureLod(irradianceAtlas, atlasUV, 0).rgb;
			float dist = length(gbufferData.ws_coords - probePos);
			diffuseIndirect += (dist / distSum) * diffuseSample;
		}
	}

	// Specular GI
	// Select one radiance probe.
	int localSpecularIndex = -1;
	for (uint i = 0; i < ubo.numRadianceProbes; ++i) {
		RadianceProbe probe = ubo.localRadianceProbes[i];
		float dist = length(gbufferData.ws_coords - probe.positionWS);
		if (dist <= probe.captureRadius) {
			localSpecularIndex = int(i);
			break;
		}
	}
	vec3 specularSample;
	if (localSpecularIndex == -1) {
		specularSample = textureLod(skyRadianceProbe, R_world, roughness * ubo.skyRadianceProbeMaxLOD).rgb;
	} else {
		vec4 R4 = vec4(R_world, float(localSpecularIndex));
		specularSample = textureLod(localRadianceCubeArray, R4, roughness * ubo.radianceProbeMaxLOD).rgb;
	}
	vec2 envBRDF          = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;
	vec3 specularIndirect = specularSample * (kS * envBRDF.x + envBRDF.y);

	// Ambient occlusion
	// NOTE: Applying occlusion AFTER integrating over hemisphere
	//       is physically wrong but that's a limit of IBL approaches.
	float ssaoSample = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 finalIrradiance = ssaoSample * (kD * diffuseIndirect + specularIndirect);
	return finalIrradiance;
}

vec3 getGlobalIllumination(GBufferData gbufferData) {
	uint shadingModel = gbufferData.material_id;
	vec3 irradiance = vec3(0.0);

	if (shadingModel == MATERIAL_SHADINGMODEL_DEFAULTLIT) {
		irradiance.rgb = getImageBasedLighting(gbufferData);
	}

	return irradiance;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbufferA, gbufferB, gbufferC, gbufferData);

	vec3 irradiance = getGlobalIllumination(gbufferData);
	irradiance.rgb = max(vec3(0.0), irradiance.rgb);

	irradiance.rgb *= ubo.overallIntensity;

	outSceneColor = vec4(irradiance, 0.0);
}
