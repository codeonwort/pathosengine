#version 430 core

layout (location = 0) out uvec4 output0; // (albedo, normal)
layout (location = 1) out vec4 output1;  // (world_position, old_specular)
layout (location = 2) out vec4 output2;  // (metallic, roughness, ao, ?)

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
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
	uvec4 outvec0 = uvec4(0);
	vec4 outvec1 = vec4(0);

	vec3 albedo = uboPerObject.albedo.xyz;

	outvec0.x = packHalf2x16(albedo.xy);
	outvec0.y = packHalf2x16(vec2(albedo.z, fs_in.normal.x));
	outvec0.z = packHalf2x16(fs_in.normal.yz);
	outvec0.w = fs_in.material_id;
	outvec1.xyz = fs_in.vs_coords;
	outvec1.w = 32.0;

	output0 = outvec0;
	output1 = outvec1;
	output2 = vec4(uboPerObject.metal_roughness.x, uboPerObject.metal_roughness.y, 0.0, 0.0);
}
