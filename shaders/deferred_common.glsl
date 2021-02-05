//? #version 460 core

// should match with MAX_DIRECTIONAL_LIGHTS in render_deferred.cpp
#define MAX_DIRECTIONAL_LIGHTS     4
#define MAX_POINT_LIGHTS           8

// This don't need to match with material_id.h
#define MATERIAL_ID_NONE           0
#define MATERIAL_ID_SOLID_COLOR    1
#define MATERIAL_ID_WIREFRAME      2
#define MATERIAL_ID_TEXTURE        3
#define MATERIAL_ID_PBR            8

struct PointLight {
	// 16 bytes
	vec3  worldPosition;
	float attenuationRadius;
	// 16 bytes
	vec3  intensity;
	float falloffExponent;
	// 16 bytes
	vec3  viewPosition;
	uint  castsShadow;
};

struct DirectionalLight {
	// 16 bytes
	vec3  wsDirection;
	float padding0;
	// 16 bytes
	vec3  intensity;
	float padding1;
	// 16 bytes
	vec3  vsDirection;
	float padding2;
};

float pointLightAttenuation(PointLight L, float d) {
	return max(0.0, sign(L.attenuationRadius - d)) / (1.0 + L.falloffExponent * d * d);
}

// Position components of camera and lights are in view space
layout (std140, binding = 0) uniform UBO_PerFrame {
	mat4x4 prevViewTransform; // For reprojection
	mat4x4 prevInverseViewTransform; // For reprojection
	mat4x4 viewTransform;
	mat4x4 inverseViewTransform;
	mat3x3 viewTransform3x3;
	mat4x4 viewProjTransform;
	mat4x4 inverseProjTransform;

	vec4 projParams;
	vec4 screenResolution; // (w, h, 1/w, 1/h)
	vec4 zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))
	vec4 time; // (currentTime, ?, ?, ?)

	mat4x4 sunViewProjection[4];
	
	vec3 eyeDirection;
	vec3 eyePosition;
	vec3 ws_eyePosition;

	uint numDirLights;
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];

	uint numPointLights;
	PointLight pointLights[MAX_POINT_LIGHTS];
} uboPerFrame;

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
vec2 CubeToEquirectangular(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // inverse atan
    uv += 0.5;
    return uv;
}

float getWorldTime() { return uboPerFrame.time.x; }

vec3 getWorldPositionFromSceneDepth(vec2 screenUV, float sceneDepth) {
	//float z = sceneDepth * 2.0 - 1.0; // Use this if not Reverse-Z
	float z = sceneDepth; // clipZ is [0,1] in Reverse-Z

    vec4 clipSpacePosition = vec4(screenUV * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uboPerFrame.inverseProjTransform * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = uboPerFrame.inverseViewTransform * viewSpacePosition;

    return worldSpacePosition.xyz;
}

vec3 getViewPositionFromSceneDepth(vec2 screenUV, float sceneDepth) {
	//float z = sceneDepth * 2.0 - 1.0; // Use this if not Reverse-Z
	float z = sceneDepth; // clipZ is [0,1] in Reverse-Z

    vec4 clipSpacePosition = vec4(screenUV * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uboPerFrame.inverseProjTransform * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

vec3 getViewPositionFromWorldPosition(vec3 wPos) {
	return (uboPerFrame.viewTransform * vec4(wPos, 1.0)).xyz;
}

// Near is 0.0, Far is 1.0
float sceneDepthToLinearDepth(vec2 screenUV, float sceneDepth) {
	vec3 vPos = getViewPositionFromSceneDepth(screenUV, sceneDepth);
	float linearDepth = (-vPos.z - uboPerFrame.zRange.x) / (uboPerFrame.zRange.y - uboPerFrame.zRange.x);
	return linearDepth;
}
