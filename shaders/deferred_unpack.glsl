#version 450 core

#include "deferred_common.glsl"
#include "shadow_mapping.glsl"
#include "brdf.glsl"

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform usampler2D gbuf2;
// reserved for more gbuffers...
layout (binding = 5) uniform sampler2D ssaoMap;
layout (binding = 6) uniform sampler2DArrayShadow csm;
layout (binding = 7) uniform samplerCubeArrayShadow pointLightShadowMaps;
layout (binding = 8) uniform samplerCube irradianceMap;    // for Diffuse IBL
layout (binding = 9) uniform samplerCube prefilterEnvMap;  // for Specular IBL
layout (binding = 10) uniform sampler2D brdfIntegrationMap; // for Specular IBL

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_Unpack {
	ivec4 enabledTechniques1;
	vec4 fogColor;
	vec4 fogParams;
	float prefilterEnvMapMaxLOD;
} ubo;

// Getters for UBO
bool isShadowEnabled()    { return ubo.enabledTechniques1.x != 0; }
bool isFogEnabled()       { return ubo.enabledTechniques1.y != 0; }
vec3 getFogColor()        { return ubo.fogColor.rgb; }
float getFogBottom()      { return ubo.fogParams.x; }
float getFogTop()         { return ubo.fogParams.y; }
float getFogAttenuation() { return ubo.fogParams.z; }

struct fragment_info {
	vec3 albedo;
	vec3 normal;
	float specular_power;
	vec3 vs_coords; // in view space
	vec3 ws_coords; // in world space
	uint material_id;
	float metallic;
	float roughness;
	float ao;
	vec3 emissive;

	vec3 ws_normal;
};

void unpackGBuffer(ivec2 coord, out fragment_info fragment) {
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	uvec4 data2 = texelFetch(gbuf2, coord, 0);

	vec2 albedoZ_normalX = unpackHalf2x16(data0.y); // (albedo.z, normal.x)
	vec2 metal_roughness = unpackHalf2x16(data2.x);
	vec2 localAO_emissiveX = unpackHalf2x16(data2.y);
	vec2 emissiveYZ = unpackHalf2x16(data2.z);

	fragment.albedo         = vec3(unpackHalf2x16(data0.x), albedoZ_normalX.x);
	fragment.normal         = normalize(vec3(albedoZ_normalX.y, unpackHalf2x16(data0.z)));
	fragment.material_id    = data0.w;

	fragment.vs_coords      = data1.xyz;
	fragment.ws_coords      = vec3(uboPerFrame.inverseViewTransform * vec4(fragment.vs_coords, 1.0));
	fragment.specular_power = data1.w;

	fragment.metallic       = metal_roughness.x;
	fragment.roughness      = metal_roughness.y;
	fragment.ao             = localAO_emissiveX.x;
	fragment.emissive       = vec3(localAO_emissiveX.y, emissiveYZ.x, emissiveYZ.y);

	fragment.ws_normal      = vec3(uboPerFrame.inverseViewTransform * vec4(fragment.normal, 0.0));
}

float getShadowing(fragment_info fragment) {
	ShadowQuery query;
	query.vPos    = fragment.vs_coords;
	query.wPos    = fragment.ws_coords;
	query.vNormal = fragment.normal;
	query.wNormal = fragment.ws_normal;
	
	return getShadowingFactor(csm, query);
}

float getShadowingByPointLight(fragment_info fragment, PointLight light, int shadowMapIndex) {
	OmniShadowQuery query;
	query.shadowMapIndex    = shadowMapIndex;
	// #todo: Remove transform
	query.lightPos          = (uboPerFrame.inverseViewTransform * vec4(light.position, 1.0)).xyz;
	query.attenuationRadius = light.attenuationRadius;
	query.wPos              = fragment.ws_coords;

	return getOmniShadowingFactor(pointLightShadowMaps, query);
}

// #todo: Too old model to be deprecated.
vec3 phongShading(fragment_info fragment) {
	vec3 result = vec3(0.0);
	vec3 N = fragment.normal;

	for(uint i = 0; i < uboPerFrame.numDirLights; ++i) {
		DirectionalLight light = uboPerFrame.directionalLights[i];

		vec3 radiance = light.intensity;
		if (i == 0 && isShadowEnabled()) {
			radiance = radiance * getShadowing(fragment);
		}

		vec3 L = -light.direction;
		float cosTheta = max(0.0, dot(N, L));
		vec3 diffuse_color = radiance * (fragment.albedo * cosTheta);
		result += diffuse_color;
	}

	int omniShadowMapIndex = 0;
	for(uint i = 0; i < uboPerFrame.numPointLights; ++i) {
		PointLight light = uboPerFrame.pointLights[i];

		vec3 L = light.position - fragment.vs_coords;
		float dist = length(L);
		float attenuation = pointLightAttenuation(light, dist);
		L = normalize(L);
		vec3 R = reflect(-L, N);
		float cosTheta = max(0.0, dot(N, L));

		vec3 radiance = light.intensity;
		radiance *= attenuation;
		if (light.castsShadow != 0 && isShadowEnabled()) {
			radiance *= getShadowingByPointLight(fragment, light, omniShadowMapIndex);
			omniShadowMapIndex += 1;
		}

		vec3 specular_color = radiance * pow(max(0.0, dot(R, -uboPerFrame.eyeDirection)), fragment.specular_power);
		vec3 diffuse_color = radiance * fragment.albedo * cosTheta;
		result += diffuse_color + specular_color;
	}

	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;
	result.rgb *= ssao;

	return result;
}

vec3 CookTorranceBRDF(fragment_info fragment) {
	vec3 N = fragment.normal;
	vec3 V = normalize(uboPerFrame.eyePosition - fragment.vs_coords);

	vec3 N_world = fragment.ws_normal;
	vec3 V_world = normalize(uboPerFrame.ws_eyePosition - fragment.ws_coords);
	vec3 R = reflect(-V_world, N_world);

	vec3 albedo = fragment.albedo;
	float metallic = fragment.metallic;
	float roughness = fragment.roughness;
	float ao = fragment.ao;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(albedo, vec3(1.0)), metallic);
	
	vec3 Lo = vec3(0.0);

	for (int i = 0; i < uboPerFrame.numDirLights; ++i) {
		DirectionalLight light = uboPerFrame.directionalLights[i];

		vec3 L = -light.direction;
		vec3 H = normalize(V + L);

		vec3 radiance = light.intensity;
		// #todo: Support shadow by each directional light
		if (i == 0 && isShadowEnabled()) {
			radiance *= getShadowing(fragment);
		}

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
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	int omniShadowMapIndex = 0;
	for (int i = 0; i < uboPerFrame.numPointLights; ++i) {
		PointLight light = uboPerFrame.pointLights[i];

		vec3 L = normalize(light.position - fragment.vs_coords);
		vec3 H = normalize(V + L);
		float distance = length(light.position - fragment.vs_coords);
		float attenuation = pointLightAttenuation(light, distance);

		vec3 radiance = light.intensity;
		radiance *= attenuation;
		if (light.castsShadow != 0 && isShadowEnabled()) {
			radiance *= getShadowingByPointLight(fragment, light, omniShadowMapIndex);
			omniShadowMapIndex += 1;
		}

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
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = texture(irradianceMap, N_world).rgb;
	vec3 diffuse    = irradiance * albedo;

	vec3 prefilteredColor = textureLod(prefilterEnvMap, R, roughness * ubo.prefilterEnvMapMaxLOD).rgb;
	vec2 envBRDF  = texture(brdfIntegrationMap, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

	vec3 ambient    = (kD * diffuse + specular) * ao;

	vec3 finalColor = ambient + Lo;

	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;
	finalColor *= ssao;

	return finalColor;
}

vec4 calculateShading(fragment_info fragment) {
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if(fragment.material_id == MATERIAL_ID_TEXTURE) {
		result.rgb = phongShading(fragment);
	} else if(fragment.material_id == MATERIAL_ID_WIREFRAME) {
		result.rgb = fragment.albedo;
	} else if(fragment.material_id == MATERIAL_ID_SOLID_COLOR || fragment.material_id == MATERIAL_ID_PBR) {
		result.rgb = CookTorranceBRDF(fragment);
	} else {
		discard;
	}
	return result;
}

// From HLSL Development Cookbook
// #todo: better fog implementation
vec3 applyFog(fragment_info fragment, vec3 color) {
	const float magic_number = getFogAttenuation(); // proper physical meaning?
	float z = length(fragment.vs_coords) * magic_number;
	float de = 0.025 * smoothstep(0.0, 6.0, getFogBottom() - fragment.ws_coords.y);
	float di = 0.045 * smoothstep(0.0, 40.0, getFogTop() - fragment.ws_coords.y);
	float extinction = exp(-z * de);
	float inscattering = exp(-z * di);
	return color * extinction + getFogColor() * (1.0 - inscattering);
}

void main() {
	fragment_info fragment;
	unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);

	vec4 luminance = calculateShading(fragment);

	if (isFogEnabled()) {
		luminance.rgb = applyFog(fragment, luminance.rgb);
	}

// shadow_mapping.glsl
#if DEBUG_CSM_ID
	luminance.rgb = vec3(getShadowing(fragment));
#endif

	// output: standard shading
	out_color = luminance;
	out_color.rgb += fragment.emissive;

	// #todo-shader: Write real opacity. Output this value in another place for depth-of-field.
	// Continue to depth_of_field.glsl
	out_color.a = -fragment.vs_coords.z;
}
