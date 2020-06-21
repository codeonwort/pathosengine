#version 450 core

#include "deferred_common_fs.glsl"

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	vec3 diffuseColor;
} uboPerObject;

in VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} fs_in;

void main() {
	vec3 albedo = uboPerObject.diffuseColor;
	vec3 normal = normalize(fs_in.normal);

	packGBuffer(albedo, normal, fs_in.material_id, fs_in.vs_coords, 0.0, 0.0, 0.0);
}
