#version 450 core

#include "deferred_common_fs.glsl"

layout (binding = 0) uniform sampler2D tex_diffuse;

in VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} fs_in;

void main() {
	vec3 albedo = texture(tex_diffuse, fs_in.texcoord).rgb;
	vec3 normal = normalize(fs_in.normal);

	packGBuffer(albedo, normal, fs_in.material_id, fs_in.vs_coords, 0.0, 0.0, 0.0, vec3(0.0));
}
