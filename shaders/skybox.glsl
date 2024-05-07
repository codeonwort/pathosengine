#version 460 core

#if VERTEX_SHADER
	#define Interpolants out
#elif FRAGMENT_SHADER
	#define Interpolants in
#endif

Interpolants SkyboxInterpolants {
	vec3 cubeDir;
} interpolants;

layout (std140, binding = 1) uniform UBO_Skybox {
	mat4 viewProj;
	float skyboxLOD;
	float intensityMultiplier;
} ubo;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

void main() {
	interpolants.cubeDir = position;
	gl_Position = (ubo.viewProj * vec4(position, 1.0)).xyww;
#if REVERSE_Z
	gl_Position.z = 0.0;
#endif
}

#endif // VERTEX_SHADER

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

#include "core/transform.glsl"

layout (binding = 0) uniform samplerCube texCube;

layout (location = 0) out vec4 outSceneColor;

void main() {
	vec4 skySample = textureLod(texCube, interpolants.cubeDir, ubo.skyboxLOD);
	outSceneColor = ubo.intensityMultiplier * skySample;
}

#endif // FRAGMENT_SHADER
