// Visualize scene depth

#version 430 core

#include "deferred_common.glsl"

// Should match with 'r.viewmode'
#define VIEWMODE_SCENEDEPTH  1
#define VIEWMODE_ALBEDO      2
#define VIEWMODE_WORLDNORMAL 3
#define VIEWMODE_METALLIC    4
#define VIEWMODE_ROUGHNESS   5
#define VIEWMODE_EMISSIVE    6
#define VIEWMODE_SSAO        7

in VS_OUT {
	vec2 screenUV;
} vs_in;

layout (std140, binding = 1) uniform UBO_VisualizeBuffer {
	int viewmode;
} ubo;

layout (binding = 0) uniform sampler2D sceneDepth;
layout (binding = 1) uniform usampler2D gbuf0;
layout (binding = 2) uniform sampler2D gbuf1;
layout (binding = 3) uniform usampler2D gbuf2;
layout (binding = 4) uniform sampler2D ssaoMap;

layout (location = 0) out vec4 outColor;

//////////////////////////////////////////////////////
// #todo-debugview: Duplicate with deferred_unpack.glsl
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
//////////////////////////////////////////////////////

void main() {
	vec2 screenUV = vs_in.screenUV;
	int viewmode = ubo.viewmode;
	outColor = vec4(0.5, 0.5, 0.5, 1.0);

	fragment_info fragmentInfo;
	unpackGBuffer(ivec2(gl_FragCoord.xy), fragmentInfo);

	if (viewmode == VIEWMODE_SCENEDEPTH) {
		float depth = texture(sceneDepth, screenUV).r;
		float linearDepth = sceneDepthToLinearDepth(vs_in.screenUV, depth);
		outColor = vec4(vec3(linearDepth), 1.0);
	} else if (viewmode == VIEWMODE_ALBEDO) {
		outColor = vec4(fragmentInfo.albedo, 1.0);
	} else if (viewmode == VIEWMODE_WORLDNORMAL) {
		outColor = vec4((fragmentInfo.ws_normal + vec3(1.0)) * 0.5, 1.0);
	} else if (viewmode == VIEWMODE_METALLIC) {
		outColor = vec4(vec3(fragmentInfo.metallic), 1.0);
	} else if (viewmode == VIEWMODE_ROUGHNESS) {
		outColor = vec4(vec3(fragmentInfo.roughness), 1.0);
	} else if (viewmode == VIEWMODE_EMISSIVE) {
		outColor = vec4(fragmentInfo.emissive, 1.0);
	} else if (viewmode == VIEWMODE_SSAO) {
		float ssao = texture2D(ssaoMap, screenUV).r;
		outColor = vec4(vec3(ssao), 1.0);
	}
}
