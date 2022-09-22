#version 460 core

#include "deferred_common.glsl"

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat3 mvTransform3x3;
	vec4 billboardParam;
} uboPerObject;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

void main() {
	// #todo-material: See deferred_pack_solidcolor_vs.glsl
	vec3 delta = vec3(0.0);
	vec4 posWS;
	if (uboPerObject.billboardParam.x > 0.0f) {
		vec3 right = inverse(uboPerObject.mvTransform3x3) * vec3(1.0, 0.0, 0.0);
		delta = right * uboPerObject.billboardParam.y * (uv.x - 0.5);
		posWS = uboPerObject.mvTransform * vec4(position + delta, 1.0);
	} else {
		posWS = uboPerObject.mvTransform * vec4(position, 1.0);
	}

	gl_Position = uboPerFrame.projTransform * posWS;
}

#endif

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

void main() {
}

#endif
