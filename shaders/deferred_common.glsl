//? #version 460 core

// UBO binding slots reserved globally.
// Each pass should use binding slots after these.
#define SLOT_UBO_PER_FRAME         0

// Should match with MAX_DIRECTIONAL_LIGHTS in render_deferred.cpp
// #todo-light: Support unlimit number of light sources
#define MAX_DIRECTIONAL_LIGHTS     4
#define MAX_POINT_LIGHTS           8

// #todo-material: Rename to MATERIAL_DOMAIN
// This don't need to match with material_id.h
#define MATERIAL_ID_NONE           0
#define MATERIAL_ID_SOLID_COLOR    1
#define MATERIAL_ID_WIREFRAME      2
#define MATERIAL_ID_TEXTURE        3
#define MATERIAL_ID_ALPHAONLY      7
#define MATERIAL_ID_PBR            8

// Total 48 bytes
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

float pointLightAttenuation(PointLight L, float d) {
	return max(0.0, sign(L.attenuationRadius - d)) / (1.0 + L.falloffExponent * d * d);
}

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
	vec3 eyePosition;    // view space
	vec3 ws_eyePosition; // world space

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
