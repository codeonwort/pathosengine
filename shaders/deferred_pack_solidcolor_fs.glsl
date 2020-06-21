#version 450 core

#include "deferred_common_fs.glsl"

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	mat3 mvTransform3x3;
	vec4 albedo;
	vec4 metal_roughness;
} uboPerObject;

in VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} fs_in;

void main() {
	vec3 albedo = uboPerObject.albedo.xyz;
	vec3 normal = normalize(fs_in.normal);
	float metallic = uboPerObject.metal_roughness.x;
	float roughness = uboPerObject.metal_roughness.y;
	float localAO = 1.0;

	packGBuffer(albedo, normal, fs_in.material_id, fs_in.vs_coords, metallic, roughness, localAO);
}
