#version 450 core

#include "deferred_common_fs.glsl"

#if !defined(TRIPLANAR_MAPPING)
	#define TRIPLANAR_MAPPING 0
#endif

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

#if TRIPLANAR_MAPPING
	// #todo: Same process for metallic, roughness, and ao
	// #todo: Apply at least model transform
	vec3 P = fs_in.ls_coords * 2.0;
	vec3 N = fs_in.normal;
	vec3 NN = N * N;
	vec3 albedoX = texture(tex_albedo, P.zy).rgb;
	vec3 albedoY = texture(tex_albedo, P.zx).rgb;
	vec3 albedoZ = texture(tex_albedo, P.xy).rgb;
	vec3 albedo = (albedoX * NN.x) + (albedoY * NN.y) + (albedoZ * NN.z);

	vec3 normalX = getNormal(N, fs_in.tangent, fs_in.bitangent, P.zy);
	vec3 normalY = getNormal(N, fs_in.tangent, fs_in.bitangent, P.zx);
	vec3 normalZ = getNormal(N, fs_in.tangent, fs_in.bitangent, P.xy);
	vec3 normal = normalize((normalX * NN.x) + (normalY * NN.y) + (normalZ * NN.z));
#else
	vec4 albedoAndOpacity = texture(tex_albedo, uv).rgba;
	vec3 albedo = albedoAndOpacity.rgb;

	// #todo: Exclude this logic for fully opaque materials.
	// Due to lack of fancy material system I'm doing just dynamic branching.
	if (shouldDiscard(albedoAndOpacity.a)) {
		discard;
	}

	vec3 normal = getNormal(fs_in.normal, fs_in.tangent, fs_in.bitangent, uv);
#endif
	
	vec3 metallic = texture(tex_metallic, uv).rgb;
	vec3 roughness = texture(tex_roughness, uv).rgb;
	vec3 ao = texture(tex_ao, uv).rgb;

	packGBuffer(albedo, normal, fs_in.material_id, fs_in.vs_coords, metallic.r, roughness.r, ao.r, vec3(0.0));
}
