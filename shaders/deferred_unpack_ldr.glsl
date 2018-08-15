#version 430 core

#include "deferred_common.glsl"

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D gbuf1;
layout (binding = 2) uniform sampler2D gbuf2;

layout (binding = 6) uniform sampler2DShadow sunDepthMap;

struct fragment_info {
	vec3 color;
	vec3 normal;
	float specular_power;
	vec3 vs_coords; // in view space
	vec3 ws_coords; // in world space
	uint material_id;
};

void unpackGBuffer(ivec2 coord, out fragment_info fragment) {
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	vec2 temp = unpackHalf2x16(data0.y);

	fragment.color = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.normal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	fragment.material_id = data0.w;

	fragment.vs_coords = data1.xyz;
	fragment.ws_coords = vec3(uboPerFrame.inverseViewTransform * vec4(fragment.vs_coords, 1.0));
	fragment.specular_power = data1.w;
}

float getShadowing(fragment_info fragment) {
	// linear depth in view space
	float vz = -fragment.vs_coords.z;
	vz = (vz - uboPerFrame.zRange.x) / (uboPerFrame.zRange.y - uboPerFrame.zRange.x);

	int mapIx = int(vz * 4.0);
	if(mapIx >= 4) {
		return 0.0;
	}

	float u0 = float(mapIx) * 0.25;

	vec4 ls_coords = uboPerFrame.sunViewProjection[mapIx] * vec4(fragment.ws_coords, 1.0);
	ls_coords.xyz = (ls_coords.xyz + vec3(1.0)) * 0.5;
	float NdotL = dot(fragment.normal, -uboPerFrame.dirLightDirs[0]);
	float bias = max(0.05 * (1.0 - NdotL), 0.005);
	float inv_w = 1.0 / ls_coords.w;
	ls_coords.xy = ls_coords.xy * inv_w;
	ls_coords.z = (ls_coords.z - bias) * inv_w;

	vec2 dudv = 1.0 / vec2(textureSize(sunDepthMap, 0));
	dudv.x *= 0.25;

	float shadow = 0.0f;
	for(int x = -1; x <= 1; ++x) {
		for(int y = -1; y <= 1; ++y) {
			vec3 uvw = ls_coords.xyz;
			uvw.x = u0 + 0.25 * uvw.x;
			uvw.xy += dudv * vec2(x, y);
			shadow += texture(sunDepthMap, uvw);
		}
	}
	shadow /= 9.0;
	
	return max(0.5, shadow);
}

vec4 calculateShading(fragment_info fragment) {
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	if(fragment.material_id != 0){
		vec3 N = fragment.normal;
		for(uint i = 0; i < uboPerFrame.numDirLights; ++i) {
			vec3 L = -uboPerFrame.dirLightDirs[i];
			float cosTheta = max(0.0, dot(N, L));
			vec3 diffuse_color = uboPerFrame.dirLightColors[i] * fragment.color * cosTheta;
			result += vec4(diffuse_color, 0.0);
		}
		for(uint i = 0; i < uboPerFrame.numPointLights; ++i) {
			vec3 L = uboPerFrame.pointLightPos[i] - fragment.vs_coords;
			float dist = length(L);
			float attenuation = 500.0 / (pow(dist, 2.0) + 1.0);
			L = normalize(L);
			vec3 R = reflect(-L, N);
			float cosTheta = max(0.0, dot(N, L));
			vec3 specular_color = uboPerFrame.pointLightColors[i] * pow(max(0.0, dot(R, -uboPerFrame.eyeDirection)), fragment.specular_power);
			vec3 diffuse_color = uboPerFrame.pointLightColors[i] * fragment.color * cosTheta;
			result += vec4(attenuation * (diffuse_color + specular_color), 0.0);
		}
	} else {
		discard;
	}
	result.rgb = result.rgb * getShadowing(fragment);
	return result;
}

void main() {
	fragment_info fragment;
	unpackGBuffer(ivec2(gl_FragCoord.xy), fragment);
	out_color = calculateShading(fragment);
}