//? #version 460 core
// light.glsl

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
