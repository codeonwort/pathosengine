#version 460 core

#include "deferred_common.glsl"

layout (std140, binding = 1) uniform UBO_Silhouette {
	mat4 modelTransform;
	vec3 color;
} ubo;

/////////////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 inPosition;

void main() {
	mat4 model = ubo.modelTransform;
	mat4 viewProj = uboPerFrame.viewProjTransform;

	vec4 positionWS = model * vec4(inPosition, 1.0);

	vec4 positionCS = viewProj * positionWS;
	positionCS.xy += uboPerFrame.temporalJitter.xy * positionCS.w;

	gl_Position = positionCS;
}

#endif

/////////////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

layout (location = 0) out vec4 outColor;

void main() {
	outColor = vec4(ubo.color, 1.0);
}

#endif
