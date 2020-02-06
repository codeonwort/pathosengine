
// should match with MeshDeferredRenderPass_Unpack::MAX_DIRECTIONAL_LIGHTS
#define MAX_DIRECTIONAL_LIGHTS     4
#define MAX_POINT_LIGHTS           8

// This don't need to match with material_id.h
#define MATERIAL_ID_NONE           0
#define MATERIAL_ID_SOLID_COLOR    1
#define MATERIAL_ID_WIREFRAME      2
#define MATERIAL_ID_TEXTURE        3
#define MATERIAL_ID_PBR            8

struct PointLight {
	vec3  position;
	float attenuationRadius;
	vec3  intensity;
	float falloffExponent;
};

struct DirectionalLight {
	vec3  direction;
	float padding0;
	vec3  intensity;
	float padding1;
};

float pointLightAttenuation(PointLight L, float d) {
	return max(0.0, sign(L.attenuationRadius - d)) / (1.0 + L.falloffExponent * d * d);
}

// in view space
layout (std140, binding = 0) uniform UBO_PerFrame {
	mat4x4 viewTransform;
	mat4x4 inverseViewTransform;
	mat3x3 viewTransform3x3;
	mat4x4 viewProjTransform;
	vec4 projParams;

	vec4 screenResolution; // (w, h, 1/w, 1/h)
	vec4 zRange; // (near, far, fovYHalf_radians, aspectRatio(w/h))

	mat4x4 sunViewProjection[4];
	
	vec3 eyeDirection;
	vec3 eyePosition;

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
