
// should match with MeshDeferredRenderPass_Unpack::MAX_DIRECTIONAL_LIGHTS
#define MAX_DIRECTIONAL_LIGHTS 8
#define MAX_POINT_LIGHTS 16

// This don't need to match with material_id.h
#define MATERIAL_ID_NONE           0
#define MATERIAL_ID_SOLID_COLOR    1
#define MATERIAL_ID_WIREFRAME      2
#define MATERIAL_ID_TEXTURE        3
#define MATERIAL_ID_PBR            8

// in view space
layout (std140, binding = 0) uniform UBO_PerFrame {
	mat4x4 viewTransform;
	mat4x4 inverseViewTransform;
	mat3x3 viewTransform3x3;
	mat4x4 viewProjTransform;
	vec4 projParams;

	vec4 screenResolution; // (w, h, 1/w, 1/h)
	vec4 zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))

	mat4 sunViewProjection[4];
	
	vec3 eyeDirection;
	vec3 eyePosition;

	uint numDirLights;
	vec3 dirLightDirs[MAX_DIRECTIONAL_LIGHTS];
	vec3 dirLightColors[MAX_DIRECTIONAL_LIGHTS];

	uint numPointLights;
	vec3 pointLightPos[MAX_POINT_LIGHTS];
	vec3 pointLightColors[MAX_POINT_LIGHTS];
} uboPerFrame;
