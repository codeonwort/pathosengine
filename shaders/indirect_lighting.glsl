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

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_IndirectLighting {
	float radianceProbeMaxLOD;
	float overallIntensity;
	uint numRadianceProbes;
	uint _pad0;
	RadianceProbe localRadianceProbes[MAX_RADIANCE_PROBES];
} ubo;

layout (binding = 0) uniform usampler2D       gbufferA;
layout (binding = 1) uniform sampler2D        gbufferB;
layout (binding = 2) uniform usampler2D       gbufferC;
layout (binding = 3) uniform sampler2D        ssaoMap;
layout (binding = 4) uniform samplerCube      skyIrradianceProbe;     // Sky diffuse IBL
layout (binding = 5) uniform samplerCube      skyRadianceProbe;       // Sky specular IBL
layout (binding = 6) uniform sampler2D        brdfIntegrationMap;     // Precomputed table for specular IBL
layout (binding = 7) uniform samplerCubeArray localRadianceCubeArray; // local specular IBLs

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
	vec3 R = reflect(-V_world, N_world);

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
	vec3 diffuseIndirect = texture(skyIrradianceProbe, N_world).rgb * albedo;

	// Specular GI
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
		specularSample = textureLod(skyRadianceProbe, R, roughness * ubo.radianceProbeMaxLOD).rgb;
	} else {
		vec4 R4 = vec4(R, float(localSpecularIndex));
		specularSample = textureLod(localRadianceCubeArray, R4, roughness * ubo.radianceProbeMaxLOD).rgb;
	}
	vec2 envBRDF          = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;
	vec3 specularIndirect = specularSample * (kS * envBRDF.x + envBRDF.y);

	// Ambient occlusion
	// NOTE: Applying occlusion AFTER integrating over hemisphere
	//       is physically wrong but that's a limit of IBL approaches.
	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 irradiance = ssao * (kD * diffuseIndirect + specularIndirect);
	return irradiance;
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
