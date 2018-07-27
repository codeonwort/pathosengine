#version 430 core

#include "deferred_common.glsl"

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;

out VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} vs_out;

uniform mat4 mvTransform;
uniform mat4 mvpTransform;

void main() {
	vs_out.vs_coords = (mvTransform * vec4(position, 1.0f)).xyz;
	vs_out.normal = mat3(mvTransform) * normal;
	vs_out.tangent = vec3(0);
	vs_out.texcoord = vec2(0);
	vs_out.material_id = MATERIAL_ID_SOLID_COLOR;

	gl_Position = mvpTransform * vec4(position, 1.0f);
}