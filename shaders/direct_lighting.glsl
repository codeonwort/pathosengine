#version 460 core

#include "deferred_common.glsl"
#include "shadow_mapping.glsl"
#include "brdf.glsl"

layout (location = 0) out vec4 outSceneColor;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform usampler2D gbuf2;
// reserved for more gbuffers...
layout (binding = 5) uniform sampler2D ssaoMap;
layout (binding = 6) uniform sampler2DArrayShadow csm;
layout (binding = 7) uniform samplerCubeArrayShadow pointLightShadowMaps;

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_DirectLighting {
	ivec4 enabledTechniques1;
	vec4 fogColor;
	vec4 fogParams;
} ubo;

// Getters for UBO
bool isShadowEnabled()    { return ubo.enabledTechniques1.x != 0; }
bool isFogEnabled()       { return ubo.enabledTechniques1.y != 0; }
vec3 getFogColor()        { return ubo.fogColor.rgb; }
float getFogBottom()      { return ubo.fogParams.x; }
float getFogTop()         { return ubo.fogParams.y; }
float getFogAttenuation() { return ubo.fogParams.z; }

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

	return getOmniShadowingFactor(pointLightShadowMaps, query);
}

// #todo: Too old model to be deprecated.
vec3 phongShading(GBufferData gbufferData) {
	vec3 result = vec3(0.0);
	vec3 N = gbufferData.normal;

	for(uint i = 0; i < uboPerFrame.numDirLights; ++i) {
		DirectionalLight light = uboPerFrame.directionalLights[i];

		vec3 radiance = light.intensity;
		if (i == 0 && isShadowEnabled()) {
			radiance = radiance * getShadowing(gbufferData);
		}

		vec3 L = -light.vsDirection;
		float cosTheta = max(0.0, dot(N, L));
		vec3 diffuse_color = radiance * (gbufferData.albedo * cosTheta);
		result += diffuse_color;
	}

	int omniShadowMapIndex = 0;
	for(uint i = 0; i < uboPerFrame.numPointLights; ++i) {
		PointLight light = uboPerFrame.pointLights[i];

		vec3 L = light.viewPosition - gbufferData.vs_coords;
		float dist = length(L);
		float attenuation = pointLightAttenuation(light, dist);
		L = normalize(L);
		vec3 R = reflect(-L, N);
		float cosTheta = max(0.0, dot(N, L));

		vec3 radiance = light.intensity;
		radiance *= attenuation;
		if (light.castsShadow != 0 && isShadowEnabled()) {
			radiance *= getShadowingByPointLight(gbufferData, light, omniShadowMapIndex);
			omniShadowMapIndex += 1;
		}

		vec3 specular_color = radiance * pow(max(0.0, dot(R, -uboPerFrame.eyeDirection)), gbufferData.specular_power);
		vec3 diffuse_color = radiance * gbufferData.albedo * cosTheta;
		result += diffuse_color + specular_color;
	}

	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;
	result.rgb *= ssao;

	return result;
}

vec3 CookTorranceBRDF(GBufferData gbufferData) {
	vec3 N = gbufferData.normal;
	vec3 V = normalize(uboPerFrame.eyePosition - gbufferData.vs_coords);

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

	for (int i = 0; i < uboPerFrame.numDirLights; ++i) {
		DirectionalLight light = uboPerFrame.directionalLights[i];

		vec3 L = -light.vsDirection;
		vec3 H = normalize(V + L);

		vec3 radiance = light.intensity;
		// #todo: Support shadow by each directional light
		if (i == 0 && isShadowEnabled()) {
			radiance *= getShadowing(gbufferData);
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

		vec3 L = normalize(light.viewPosition - gbufferData.vs_coords);
		vec3 H = normalize(V + L);
		float distance = length(light.viewPosition - gbufferData.vs_coords);
		float attenuation = pointLightAttenuation(light, distance);

		vec3 radiance = light.intensity;
		radiance *= attenuation;
		if (light.castsShadow != 0 && isShadowEnabled()) {
			radiance *= getShadowingByPointLight(gbufferData, light, omniShadowMapIndex);
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

	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;

	vec3 finalRadiance = Lo * ssao;
	return finalRadiance;
}

vec4 getLocalIllumination(GBufferData gbufferData) {
	uint ID = gbufferData.material_id;

	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if (ID == MATERIAL_ID_TEXTURE) {
		result.rgb = phongShading(gbufferData);
	} else if(ID == MATERIAL_ID_WIREFRAME || ID == MATERIAL_ID_ALPHAONLY) {
		result.rgb = gbufferData.albedo;
	} else if(ID == MATERIAL_ID_SOLID_COLOR || ID == MATERIAL_ID_PBR) {
		result.rgb = CookTorranceBRDF(gbufferData);
	} else {
		discard;
	}
	return result;
}

// From HLSL Development Cookbook
// #todo: better height fog implementation
vec3 applyFog(GBufferData gbufferData, vec3 color) {
	const float magic_number = getFogAttenuation(); // proper physical meaning?
	float z = length(gbufferData.vs_coords) * magic_number;
	float de = 0.025 * smoothstep(0.0, 6.0, getFogBottom() - gbufferData.ws_coords.y);
	float di = 0.045 * smoothstep(0.0, 40.0, getFogTop() - gbufferData.ws_coords.y);
	float extinction = exp(-z * de);
	float inscattering = exp(-z * di);
	return color * extinction + getFogColor() * (1.0 - inscattering);
}

void main() {
	GBufferData gbufferData;
	unpackGBuffer(ivec2(gl_FragCoord.xy), gbuf0, gbuf1, gbuf2, gbufferData);

	vec4 radiance = getLocalIllumination(gbufferData);

	if (isFogEnabled()) {
		radiance.rgb = applyFog(gbufferData, radiance.rgb);
	}

// shadow_mapping.glsl
#if DEBUG_CSM_ID
	radiance.rgb = vec3(getShadowing(gbufferData));
#endif

	// #todo: Sometimes radiance.rgb is NaN, which results in vec3(65536.0) by bloom setup pass,
	// which results in a big white circle by bloom pass.
	radiance.rgb = max(vec3(0.0), radiance.rgb);

	// output: standard shading
	outSceneColor = radiance;
	outSceneColor.rgb += gbufferData.emissive;

	// #todo-shader: Write real opacity. Output this value in another place for depth-of-field.
	// Continue to depth_of_field.glsl
	outSceneColor.a = -gbufferData.vs_coords.z;
}
