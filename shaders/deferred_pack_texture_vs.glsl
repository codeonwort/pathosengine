#version 430 core

#include "deferred_common.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
} uboPerObject;

out VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} vs_out;

void main() {
	vs_out.vs_coords = (uboPerObject.mvTransform * vec4(position, 1.0f)).xyz;
	vs_out.normal = mat3(uboPerObject.mvTransform) * normal;
	vs_out.tangent = vec3(0);
	vs_out.texcoord = uv;
	vs_out.material_id = MATERIAL_ID_TEXTURE;

	gl_Position = uboPerObject.mvpTransform * vec4(position, 1.0f);
}
