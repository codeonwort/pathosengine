#version 450 core

#include "deferred_common.glsl"

layout (std140, binding = 1) uniform UBO_PerObject {
	mat4 mvpTransform;
} uboPerObject;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

void main() {
	gl_Position = uboPerObject.mvpTransform * vec4(position, 1.0);
}

#endif

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

void main() {
	//
}

#endif
