#version 430 core

#include "deferred_common.glsl"
#include "shadow_mapping.glsl"
#include "brdf.glsl"

#define SOFT_SHADOW   1
#define DEBUG_CSM_ID  0

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright; // bright area only

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform sampler2D gbuf2;
layout (binding = 5) uniform sampler2D ssaoMap;
layout (binding = 6) uniform sampler2DArrayShadow csm;

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_Unpack {
	ivec4 enabledTechniques1;
	vec4 fogColor;
	vec4 fogParams;
	vec4 bloomParams;
} ubo;

// Getters for UBO
bool isShadowEnabled()    { return ubo.enabledTechniques1.x != 0; }
bool isFogEnabled()       { return ubo.enabledTechniques1.y != 0; }
vec3 getFogColor()        { return ubo.fogColor.rgb; }
float getFogBottom()      { return ubo.fogParams.x; }
float getFogTop()         { return ubo.fogParams.y; }
float getFogAttenuation() { return ubo.fogParams.z; }
float getBloomStrength()  { return ubo.bloomParams.x; }
float getMinBloom()       { return ubo.bloomParams.y; }
float getMaxBloom()       { return ubo.bloomParams.z; }

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
};

void unpackGBuffer(ivec2 coord, out fragment_info fragment) {
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	vec4 data2 = texelFetch(gbuf2, coord, 0);

	vec2 temp = unpackHalf2x16(data0.y); // (albedo.z, normal.x)

	fragment.albedo         = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.normal         = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	fragment.material_id    = data0.w;

	fragment.vs_coords      = data1.xyz;
	fragment.ws_coords      = vec3(uboPerFrame.inverseViewTransform * vec4(fragment.vs_coords, 1.0));
	fragment.specular_power = data1.w;

	fragment.metallic       = data2.x;
	fragment.roughness      = data2.y;
	fragment.ao             = data2.z;
}

float getShadowing(fragment_info fragment) {
	ShadowQuery query;
	query.vPos    = fragment.vs_coords;
	query.wPos    = fragment.ws_coords;
	query.vNormal = fragment.normal;
	
	return getShadowingFactor(csm, query);
}

vec3 phongShading(fragment_info fragment) {
	vec3 result = vec3(0.0);
	vec3 N = fragment.normal;

	for(uint i = 0; i < uboPerFrame.numDirLights; ++i) {
		DirectionalLight light = uboPerFrame.directionalLights[i];

		vec3 L = -light.direction;
		float cosTheta = max(0.0, dot(N, L));
		vec3 diffuse_color = light.intensity * (fragment.albedo * cosTheta);
		result += diffuse_color;
	}

	for(uint i = 0; i < uboPerFrame.numPointLights; ++i) {
		PointLight light = uboPerFrame.pointLights[i];

		vec3 L = light.position - fragment.vs_coords;
		float dist = length(L);
		float attenuation = pointLightAttenuation(light, dist);
		L = normalize(L);
		vec3 R = reflect(-L, N);
		float cosTheta = max(0.0, dot(N, L));
		vec3 specular_color = light.intensity * pow(max(0.0, dot(R, -uboPerFrame.eyeDirection)), fragment.specular_power);
		vec3 diffuse_color = light.intensity * fragment.albedo * cosTheta;
		result += attenuation * (diffuse_color + specular_color);
	}

	if(isShadowEnabled()) {
		result.rgb = result.rgb * getShadowing(fragment);
	}

	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;
	result.rgb *= ssao;

	return result;
}

vec3 CookTorranceBRDF(fragment_info fragment) {
	vec3 N = fragment.normal;
	vec3 V = normalize(uboPerFrame.eyePosition - fragment.vs_coords);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, min(fragment.albedo, vec3(1.0)), fragment.metallic);
	
	vec3 Lo = vec3(0.0);

	for (int i = 0; i < uboPerFrame.numDirLights; ++i) {
		DirectionalLight light = uboPerFrame.directionalLights[i];

		vec3 L = -light.direction;
		vec3 H = normalize(V + L);
		vec3 radiance = light.intensity;

		float NDF = distributionGGX(N, H, fragment.roughness);
		float G = geometrySmith(N, V, L, fragment.roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - fragment.metallic;

		vec3 num = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = num / max(denom, 0.001);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * fragment.albedo / PI + specular) * radiance * NdotL;
	}

	for (int i = 0; i < uboPerFrame.numPointLights; ++i) {
		PointLight light = uboPerFrame.pointLights[i];

		vec3 L = normalize(light.position - fragment.vs_coords);
		vec3 H = normalize(V + L);
		float distance = length(light.position - fragment.vs_coords);
		float attenuation = pointLightAttenuation(light, distance);
		vec3 radiance = light.intensity;
		radiance *= attenuation;

		float NDF = distributionGGX(N, H, fragment.roughness);
		float G = geometrySmith(N, V, L, fragment.roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - fragment.metallic;

		vec3 num = NDF * G * F;
		float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = num / max(denom, 0.001);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * fragment.albedo / PI + specular) * radiance * NdotL;
	}

	vec3 ambient = vec3(0.03) * fragment.albedo * fragment.ao;
	vec3 finalColor = ambient + Lo;

	float ssao = texture2D(ssaoMap, fs_in.screenUV).r;
	finalColor *= ssao;

	if(isShadowEnabled()) {
		finalColor = finalColor * getShadowing(fragment);
	}

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
	vec4 color = calculateShading(fragment);

	if(isFogEnabled()) {
		color.rgb = applyFog(fragment, color.rgb);
	}

#if DEBUG_CSM_ID
	color.rgb = vec3(getShadowing(fragment));
#endif

	// output: standard shading
	out_color = color;

	// #todo-shader: Write real opacity. Output this value in another place.
	// for depth-of-field. continue to blur_pass.glsl
	out_color.a = -fragment.vs_coords.z;

	// output: light bloom
	float Y = dot(color.xyz, vec3(0.299, 0.587, 0.144));
	color.xyz = color.xyz * getBloomStrength() * smoothstep(getMinBloom(), getMaxBloom(), Y);
	out_bright = color;
}
