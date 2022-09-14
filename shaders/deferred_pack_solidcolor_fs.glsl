#version 460 core

#include "deferred_common_fs.glsl"

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	mat3 mvTransform3x3;
	vec4 albedo_metal;
	vec4 emissive_roughness;
	vec4 billboardParam;
} uboPerObject;

in VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} fs_in;

void main() {
	vec3 albedo = uboPerObject.albedo_metal.xyz;
	vec3 normal = normalize(fs_in.normal);
	float metallic = uboPerObject.albedo_metal.w;
	vec3 emissive = uboPerObject.emissive_roughness.xyz;
	float roughness = uboPerObject.emissive_roughness.w;
	float localAO = 1.0;

	packGBuffer(albedo, normal, fs_in.material_id, fs_in.vs_coords, metallic, roughness, localAO, emissive);
}
