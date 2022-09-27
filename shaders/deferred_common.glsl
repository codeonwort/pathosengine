//? #version 460 core

// --------------------------------------------------------
// Global constants

// UBO binding slots reserved globally.
// Each pass should use binding slots after these.
#define SLOT_UBO_PER_FRAME         0

// --------------------------------------------------------
// Materials

// #todo-material: Deprecated. See common.glsl.
// UNLIT, DEFAULTLIT, SUBSURFACE, HAIR, TRANSLUCENT, ...
#define MATERIAL_ID_NONE           0
#define MATERIAL_ID_UNLIT          1
#define MATERIAL_ID_PBR            2
#define MATERIAL_ID_ALPHAONLY      4

// --------------------------------------------------------
// Lights

// Total 64 bytes
struct PointLight {
	// 16 bytes
	vec3  worldPosition;
	float attenuationRadius;
	// 16 bytes
	vec3  intensity;
	float falloffExponent;
	// 16 bytes
	vec3  positionVS;
	uint  castsShadow;
	// 16 bytes
	float sourceRadius;
	vec3  padding0;
};

// Total 48 bytes
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

// Total 80 bytes
struct RectLight {
	// 16 bytes
	vec3 positionVS;
	float attenuationRadius;
	// 16 bytes
	vec3 directionVS;
	uint castsShadow;
	// 16 bytes
	vec3 intensity;
	float falloffExponent;
	// 16 bytes
	vec3 upVS;
	float halfHeight;
	// 16 bytes
	vec3 rightVS;
	float halfWidth;
};

// SIGGRAPH 2013: Real Shading in Unreal Engine 4 by Brian Karis, Epic Games
// r: light's attenuation radius
// d: distance
float pointLightFalloff(float r, float d) {
	float num = d / r;
	num = num * num;
	num = num * num;
	num = 1.0 - num;
	num = clamp(num, 0.0, 1.0);
	num = num * num;

	float denom = 1.0 + d * d;

	return num / denom;
}

// --------------------------------------------------------
// Uniform buffers

// #todo: Rename parameters to clarify view space and world space values.
// Position components of camera and lights are in view space
layout (std140, binding = SLOT_UBO_PER_FRAME) uniform UBO_PerFrame {
	mat4x4 prevViewTransform; // For reprojection
	mat4x4 prevInverseViewTransform; // For reprojection
	mat4x4 viewTransform;
	mat4x4 inverseViewTransform;
	mat3x3 viewTransform3x3;
	mat4x4 viewProjTransform;
	mat4x4 projTransform;
	mat4x4 inverseProjTransform;

	vec4 projParams;
	vec4 screenResolution; // (w, h, 1/w, 1/h)
	vec4 zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))
	vec4 time; // (currentTime, ?, ?, ?)

	mat4x4 sunViewProjection[4];
	
	vec3 eyeDirection;   // view space
	float _padding0;

	vec3 eyePosition;    // view space
	float _padding1;

	vec3 ws_eyePosition; // world space
	uint sunExists;

	DirectionalLight sunLight;
} uboPerFrame;

// --------------------------------------------------------
// Math utils

// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
vec2 CubeToEquirectangular(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); // inverse atan
    uv += 0.5;
    return uv;
}

float getWorldTime() { return uboPerFrame.time.x; }
float getAspectRatio() { return uboPerFrame.zRange.w; }

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

// zNear is mapped to 0.0, zFar is mapped to 1.0
float sceneDepthToLinearDepth(vec2 screenUV, float sceneDepth) {
	vec3 vPos = getViewPositionFromSceneDepth(screenUV, sceneDepth);
	float linearDepth = (-vPos.z - uboPerFrame.zRange.x) / (uboPerFrame.zRange.y - uboPerFrame.zRange.x);
	return linearDepth;
}

// --------------------------------------------------------
// GBuffers

// GBuffer unpack info
struct GBufferData {
	vec3 albedo;
	vec3 normal;          // Vertex normal in view space
	float specular_power; // #todo: Deprecated property for phong shading
	vec3 vs_coords;       // Position in view space
	vec3 ws_coords;       // Position in world space
	uint material_id;     // Shading model ID
	float metallic;
	float roughness;
	float ao;             // material local AO
	vec3 emissive;

	vec3 ws_normal;
};

void unpackGBuffer(
	ivec2 coord, usampler2D gbufferA, sampler2D gbufferB, usampler2D gbufferC,
	out GBufferData outGBufferData)
{
	uvec4 data0 = texelFetch(gbufferA, coord, 0);
	vec4 data1 = texelFetch(gbufferB, coord, 0);
	uvec4 data2 = texelFetch(gbufferC, coord, 0);

	vec2 albedoZ_normalX = unpackHalf2x16(data0.y); // (albedo.z, normal.x)
	vec2 metal_roughness = unpackHalf2x16(data2.x);
	vec2 localAO_emissiveX = unpackHalf2x16(data2.y);
	vec2 emissiveYZ = unpackHalf2x16(data2.z);

	outGBufferData.albedo         = vec3(unpackHalf2x16(data0.x), albedoZ_normalX.x);
	outGBufferData.normal         = normalize(vec3(albedoZ_normalX.y, unpackHalf2x16(data0.z)));
	outGBufferData.material_id    = data0.w;

	outGBufferData.vs_coords      = data1.xyz;
	outGBufferData.ws_coords      = vec3(uboPerFrame.inverseViewTransform * vec4(outGBufferData.vs_coords, 1.0));
	outGBufferData.specular_power = data1.w;

	outGBufferData.metallic       = metal_roughness.x;
	outGBufferData.roughness      = metal_roughness.y;
	outGBufferData.ao             = localAO_emissiveX.x;
	outGBufferData.emissive       = vec3(localAO_emissiveX.y, emissiveYZ.x, emissiveYZ.y);

	outGBufferData.ws_normal      = vec3(uboPerFrame.inverseViewTransform * vec4(outGBufferData.normal, 0.0));
}
