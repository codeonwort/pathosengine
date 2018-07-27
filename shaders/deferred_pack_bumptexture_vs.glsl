#version 430 core

#include "deferred_common.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
    vec3 bitangent;
	vec2 texcoord;
	flat uint material_id;
} vs_out;

uniform mat3 mvTransform3x3;
uniform mat4 mvTransform;
uniform mat4 mvpTransform;

void main() {
	vs_out.vs_coords = (mvTransform * vec4(position, 1.0f)).xyz;
	vs_out.normal = mvTransform3x3 * normal;
	vs_out.tangent = mvTransform3x3 * tangent;
    vs_out.bitangent = mvTransform3x3 * bitangent;
	vs_out.texcoord = uv;
	vs_out.material_id = MATERIAL_ID_TEXTURE;

	gl_Position = mvpTransform * vec4(position, 1.0f);
}