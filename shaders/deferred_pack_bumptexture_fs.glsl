#version 430 core

layout (location = 0) out uvec4 output0;
layout (location = 1) out vec4 output1;
layout (location = 2) out vec4 output2;

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
	uvec4 outvec0 = uvec4(0);
	vec4 outvec1 = vec4(0);

	vec3 color = texture(tex_diffuse, fs_in.texcoord).rgb;
    vec3 normal = getNormal(fs_in.normal, fs_in.tangent, fs_in.bitangent, fs_in.texcoord);

	outvec0.x = packHalf2x16(color.xy);
	outvec0.y = packHalf2x16(vec2(color.z, normal.x));
	outvec0.z = packHalf2x16(normal.yz);
	outvec0.w = fs_in.material_id;

	outvec1.xyz = fs_in.vs_coords;
	outvec1.w = 128.0;

	output0 = outvec0;
	output1 = outvec1;
	output2 = vec4(0.0);
}
