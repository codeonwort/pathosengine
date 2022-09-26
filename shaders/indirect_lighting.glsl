#version 460 core

#include "deferred_common.glsl"
#include "brdf.glsl"

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_IndirectLighting {
	float prefilterEnvMapMaxLOD;
} ubo;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform usampler2D gbuf2;
layout (binding = 3) uniform sampler2D ssaoMap;
layout (binding = 4) uniform samplerCube irradianceMap;    // Diffuse IBL
layout (binding = 5) uniform samplerCube prefilterEnvMap;  // Specular IBL
layout (binding = 6) uniform sampler2D brdfIntegrationMap; // Specular IBL

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

vec3 getIBL(GBufferData gbufferData) {
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
	vec3 diffuseIndirect = texture(irradianceMap, N_world).rgb * albedo;

	// Specular GI
	vec3 prefilteredColor = textureLod(prefilterEnvMap, R, roughness * ubo.prefilterEnvMapMaxLOD).rgb;
	vec2 envBRDF  = texture(brdfIntegrationMap, vec2(NdotV, roughness)).rg;
	vec3 specularIndirect = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

	// Ambient occlusion
	// NOTE: Applying occlusion AFTER integrating over hemisphere
	//       is physically wrong but that's a limit of an IBL approach.
	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 irradiance = ssao * (kD * diffuseIndirect + specularIndirect);
	return irradiance;
}

vec3 getGlobalIllumination(GBufferData gbufferData) {
	uint ID = gbufferData.material_id;
	vec3 irradiance = vec3(0.0);

	if (ID == MATERIAL_ID_PBR) {
		irradiance.rgb = getIBL(gbufferData);
	}

	return irradiance;
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbuf0, gbuf1, gbuf2, gbufferData);

	vec3 irradiance = getGlobalIllumination(gbufferData);
	irradiance.rgb = max(vec3(0.0), irradiance.rgb);

	outSceneColor = vec4(irradiance, 0.0);
}
