#version 460 core

#include "core/transform.glsl"
#include "deferred_common.glsl"

// --------------------------------------------------------
// Input

layout (std140, binding = 1) uniform UBO_VolumetricCloudPost {
	uint  bPanorama;
	float zFar;
	uint  _pad1;
	uint  _pad2;
} uboCloudPost;

in VS_OUT {
	vec2 screenUV;
} interpolants;

layout (binding = 0) uniform sampler2D volumetricClouds;
layout (binding = 1) uniform sampler2D sceneDepth;

// --------------------------------------------------------
// Output

layout (location = 0) out vec4 outSceneColor;

// --------------------------------------------------------
// Shader

vec3 getViewDirection(vec2 uv) {
	vec3 P = vec3(2.0 * uv - 1.0, 0.0);
	P.x *= getAspectRatio();
	P.z = -(1.0 / tan(getFOV() * 0.5));
	P = normalize(P);

	mat3 camera_transform = mat3(uboPerFrame.inverseViewTransform);
	vec3 ray_forward = camera_transform * P;

	return ray_forward;
}

void main() {
	vec2 screenUV = interpolants.screenUV;

	vec3 dir = getViewDirection(screenUV);
	vec2 panoramaUV = CubeToEquirectangular(dir);

	bool bPanorama = (uboCloudPost.bPanorama != 0);

	float deviceZ = texelFetch(sceneDepth, ivec2(gl_FragCoord.xy), 0).r;
	bool bShouldClip = bPanorama && deviceZ != uboCloudPost.zFar;

	vec2 uv = bPanorama ? panoramaUV : screenUV;
	vec4 color = textureLod(volumetricClouds, uv, 0);

	outSceneColor = bShouldClip ? vec4(0,0,0,1) : color;
}
