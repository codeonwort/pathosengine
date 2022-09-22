#version 460 core

#include "deferred_common.glsl"

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat4 mvpTransform;
	vec3 diffuseColor;
} uboPerObject;

out VS_OUT {
	vec3 vs_coords;
	vec3 normal;
	vec3 tangent;
	vec2 texcoord;
	flat uint material_id;
} vs_out;

void main() {
	vs_out.vs_coords   = (uboPerObject.mvTransform * vec4(position, 1.0f)).xyz;
	vs_out.normal      = mat3(uboPerObject.mvTransform) * normal;
	vs_out.tangent     = vec3(0);
	vs_out.texcoord    = vec2(0);
	vs_out.material_id = MATERIAL_ID_WIREFRAME;

	gl_Position = uboPerFrame.projTransform * (uboPerObject.mvTransform * vec4(position, 1.0f));
}
