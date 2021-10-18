#version 450 core

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
} ubo;

//////////////////////////////////////////////////////////////////////////

#if VERTEX_SHADER

layout (location = 0) in vec3 position;

void main() {
	interpolants.cubeDir = position;
	gl_Position = (ubo.viewProj * vec4(position, 1)).xyww;
}

#endif // VERTEX_SHADER

//////////////////////////////////////////////////////////////////////////

#if FRAGMENT_SHADER

layout (binding = 0) uniform samplerCube texCube;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright;

vec2 CubeToEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // inverse atan
    uv += 0.5;
    return uv;
}

void main() {
  out_color = textureLod(texCube, interpolants.cubeDir, ubo.skyboxLOD);
  out_bright = vec4(0.0);
}

#endif // FRAGMENT_SHADER
