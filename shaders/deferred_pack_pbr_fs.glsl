#version 430 core

layout (location = 0) out uvec4 output0; // (albedo, normal)
layout (location = 1) out vec4 output1; // (world_position, old_specular)
layout (location = 2) out vec4 output2; // (metallic, roughness, ao, ?)

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
	uvec4 outvec0 = uvec4(0);
	vec4 outvec1 = vec4(0);
	vec4 outvec2 = vec4(0);

	vec3 albedo = texture(tex_albedo, fs_in.texcoord).rgb;
	vec3 normal = getNormal(fs_in.normal, fs_in.tangent, fs_in.bitangent, fs_in.texcoord);
	vec3 metallic = texture(tex_metallic, fs_in.texcoord).rgb;
	vec3 roughness = texture(tex_roughness, fs_in.texcoord).rgb;
	vec3 ao = texture(tex_ao, fs_in.texcoord).rgb;

	outvec0.x = packHalf2x16(albedo.xy);
	outvec0.y = packHalf2x16(vec2(albedo.z, normal.x));
	outvec0.z = packHalf2x16(normal.yz);
	outvec0.w = fs_in.material_id;

	outvec1.xyz = fs_in.vs_coords;
	outvec1.w = 128.0;

	outvec2.x = metallic.r;
	outvec2.y = roughness.r;
	outvec2.z = ao.r;

	output0 = outvec0;
	output1 = outvec1;
	output2 = outvec2;
}
