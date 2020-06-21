#version 450 core

#include "deferred_common_fs.glsl"

layout (binding = 0) uniform sampler2D tex_diffuse;
layout (binding = 1) uniform sampler2D tex_normal;

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	mat3 mvTransform3x3;
} uboPerObject;

in VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
    vec3 bitangent;
	vec2 texcoord;
	flat uint material_id;
} fs_in;

vec3 getNormal(vec3 n, vec3 t, vec3 b, vec2 uv) {
    vec3 T = uboPerObject.mvTransform3x3 * normalize(t);
    vec3 B = uboPerObject.mvTransform3x3 * normalize(b);
    vec3 N = uboPerObject.mvTransform3x3 * normalize(n);
    mat3 TBN = mat3(T, B, N);

    vec3 norm = normalize(texture2D(tex_normal, uv).rgb * 2.0 - 1.0);
    norm = TBN * norm; // into view space

    return norm;
}

void main() {
	vec3 albedo = texture(tex_diffuse, fs_in.texcoord).rgb;
    vec3 normal = getNormal(fs_in.normal, fs_in.tangent, fs_in.bitangent, fs_in.texcoord);

	packGBuffer(albedo, normal, fs_in.material_id, fs_in.vs_coords, 0.0, 0.0, 0.0);
}
