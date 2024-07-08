//? #version 460 core
// image_based_lighting.glsl

// #todo-light-probe: SH encoding for indirect diffuse

// Should match with image_based_lighting.h
struct IrradianceVolume {
	vec3  minBounds;
	uint  firstTileID;
	vec3  maxBounds;
	uint  numProbes;
	uvec3 gridSize;
	float captureRadius;
};

// Should match with image_based_lighting.h
struct ReflectionProbe {
	vec3  positionWS;
	float captureRadius;
};
