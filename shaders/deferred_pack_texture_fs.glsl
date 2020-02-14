#version 430 core

layout (location = 0) out uvec4 output0;
layout (location = 1) out vec4 output1;
layout (location = 2) out vec4 output2;

layout (binding = 0) uniform sampler2D tex_diffuse;

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

	vec3 albedo = texture(tex_diffuse, fs_in.texcoord).rgb;
	vec3 normal = normalize(fs_in.normal);

	outvec0.x = packHalf2x16(albedo.xy);
	outvec0.y = packHalf2x16(vec2(albedo.z, normal.x));
	outvec0.z = packHalf2x16(normal.yz);
	outvec0.w = fs_in.material_id;
	outvec1.xyz = fs_in.vs_coords;
	outvec1.w = 128.0;

	output0 = outvec0;
	output1 = outvec1;
	output2 = vec4(0.0);
}
