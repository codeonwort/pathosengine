
// should match with MeshDeferredRenderPass_Unpack::MAX_DIRECTIONAL_LIGHTS
#define MAX_DIRECTIONAL_LIGHTS 8
#define MAX_POINT_LIGHTS 16

// in view space
layout (std140, binding = 0) uniform UBO_PerFrame {
	mat4x4 viewTransform;
	mat4x4 viewProjTransform;
	
	vec3 eyeDirection;
	vec3 eyePosition;

	uint numDirLights;
	vec3 dirLightDirs[MAX_DIRECTIONAL_LIGHTS];
	vec3 dirLightColors[MAX_DIRECTIONAL_LIGHTS];

	uint numPointLights;
	vec3 pointLightPos[MAX_POINT_LIGHTS];
	vec3 pointLightColors[MAX_POINT_LIGHTS];
} uboPerFrame;