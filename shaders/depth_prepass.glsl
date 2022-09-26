#version 460 core

#include "deferred_common.glsl"

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvTransform;
	mat3 mvTransform3x3;
} uboPerObject;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

void main() {
	vec4 posWS = uboPerObject.mvTransform * vec4(position, 1.0);
	gl_Position = uboPerFrame.projTransform * posWS;
}

#endif

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

void main() {
}

#endif
