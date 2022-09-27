#version 460 core

#include "deferred_common_fs.glsl"

// #todo-driver-bug: What's this :/
#define WORKAROUND_RYZEN_6800U_BUG 1

layout (binding = 0) uniform sampler2D tex_albedo;
layout (binding = 1) uniform sampler2D tex_normal;
layout (binding = 2) uniform sampler2D tex_metallic;
layout (binding = 3) uniform sampler2D tex_roughness;
layout (binding = 4) uniform sampler2D tex_ao;

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	mat3 mvTransform3x3;
} uboPerObject;

in VS_OUT {
	vec3 vs_coords;
	vec3 ls_coords;
	vec3 normal;
	vec3 tangent;
    vec3 bitangent;
	vec2 texcoord;
	flat uint material_id;
} fs_in;

// Returns normal vector in view space
vec3 getNormal(vec3 n, vec3 t, vec3 b, vec2 uv) {
    vec3 T = normalize(uboPerObject.mvTransform3x3 * t);
    vec3 B = normalize(uboPerObject.mvTransform3x3 * b);
    vec3 N = normalize(uboPerObject.mvTransform3x3 * n);
    mat3 TBN = mat3(T, B, N);

    vec3 norm = normalize(texture2D(tex_normal, uv).rgb * 2.0 - 1.0);
    norm = TBN * norm;

    return norm;
}

void main() {
	vec2 uv = fs_in.texcoord;

	vec4 albedoAndOpacity = texture(tex_albedo, uv).rgba;
	vec3 albedo = albedoAndOpacity.rgb;

	// #todo: Exclude this logic for fully opaque materials.
	// Due to lack of fancy material system I'm doing just dynamic branching.
	if (shouldDiscard(albedoAndOpacity.a)) {
		discard;
	}

	vec3 normal = getNormal(fs_in.normal, fs_in.tangent, fs_in.bitangent, uv);
	
	float metallic = texture(tex_metallic, uv).r;
	float roughness = texture(tex_roughness, uv).r;
	float localAO = texture(tex_ao, uv).r;

	// #todo: Support emissive
	vec3 emissive = vec3(0.0);

	packGBuffer(
		albedo,
		normal,
		fs_in.material_id,
		fs_in.vs_coords,
		metallic,
		roughness,
		localAO,
		emissive);

#if WORKAROUND_RYZEN_6800U_BUG
	// #todo-driver-bug: Somehow the line 'out2.z = packHalf2x16(emissive.yz)'
	// in packGBuffer() is bugged only on Ryzen 6800U.
	packOutput2.z = 0;
#endif
}
