// References
// - GPU Pro 5, "Hi-Z Screen Space Cone-Traced Reflections"
// - https://sugulee.wordpress.com/2021/01/19/screen-space-reflections-implementation-and-optimization-part-2-hi-z-tracing-method/
// - http://bitsquid.blogspot.com/2017/08/notes-on-screen-space-hiz-tracing.html

#version 450 core

#include "deferred_common.glsl"

// Start tracing in this level.
#define HIZ_START_LEVEL      2
// Stop tracing if current level is higher than this. (higher level means lower value)
#define HIZ_STOP_LEVEL       0
#define MAX_ITERATIONS       48
#define MAX_THICKNESS        0.001

// --------------------------------------------------------
// Input

in VS_OUT {
	vec2 screenUV;
} fs_in;

layout (std140, binding = 1) uniform UBO_RayTracing {
	vec2 sceneSize;
	uint hiZMipCount;
} ubo;

layout (binding = 0) uniform sampler2D inSceneColor;
layout (binding = 1) uniform sampler2D inSceneDepth;
layout (binding = 2) uniform sampler2D inHiZ;
layout (binding = 3) uniform sampler2D inGBufferA;
layout (binding = 4) uniform sampler2D inGBufferB;
layout (binding = 5) uniform sampler2D inGBufferC;

// --------------------------------------------------------
// Output

layout (location = 0) out vec3 outRayTracingResult;

// --------------------------------------------------------
// Shader

vec3 intersectDepthPlane(vec3 o, vec3 d, float z) {
	return o + d * z;
}

// Index of the cell that contains the given 2D position.
ivec2 getCell(vec2 screenUV, ivec2 cellCount) {
	return ivec2(screenUV * cellCount);
}

// The number of cells in the quad tree at the given level.
ivec2 getCellCount(int level) {
	return textureSize(inHiZ, level);
}

// Returns screen space position of the intersection
// between o + d*t and the closest cell boundary at current HiZ level.
vec3 intersectCellBoundary(
	vec3 pos, vec3 dir,
	ivec2 cell, ivec2 cellCount,
	vec2 crossStep, vec2 crossOffset)
{
	vec3 intersection = vec3(0.0);

	vec2 index = cell + crossStep;
	vec2 boundary = index / vec2(cellCount); // Screen space position of the boundary
	boundary += crossOffset;

	vec2 delta = boundary - pos.xy;
	delta /= dir.xy;
	float t = min(delta.x, delta.y);

	intersection = intersectDepthPlane(pos, dir, t);
	return intersection;
}

bool crossedCellBoundary(ivec2 oldCellIx, ivec2 newCellIx) {
	return oldCellIx.x != newCellIx.x || oldCellIx.y != newCellIx.y;
	//return any(notEqual(oldCellIx, newCellIx));
}

// Minimum depth of the current cell in the current HiZ level.
float getMinimumDepthPlane(ivec2 cellIx, int level) {
	vec2 minmax = texelFetch(inHiZ, cellIx, level).xy;
	return minmax.x;
}

float getMaxTraceDistance(vec3 p, vec3 v) {
	vec3 traceDistances;
	if (v.x < 0.0) {
		traceDistances.x = p.x / (-v.x);
	} else {
		traceDistances.x = (1.0 - p.x) / v.x;
	}
	if (v.y < 0.0) {
		traceDistances.y = p.y / (-v.y);
	} else {
		traceDistances.y = (1.0 - p.y) / v.y;
	}
	if (v.z < 0.0) {
		traceDistances.z = p.z / (-v.z);
	} else {
		traceDistances.z = (1.0 - p.z) / v.z;
	}
	return min(traceDistances.x, min(traceDistances.y, traceDistances.z));
}

// p            : Screen space position
// v            : Screen space reflection direction
// hitPointSS   : Returns screen space hit point
// Return value : Whether RT actually hit a surface
bool traceHiZ(
	vec3 p, vec3 v, out vec3 hitPointSS,
	// Debug output, preferably compiled out.
	out int debugFinalLevel, out uint debugIterations)
{
	const int maxLevel = int(ubo.hiZMipCount) - 1; // Last mip level
	float maxTraceDistance = getMaxTraceDistance(p, v);

	// Get the cell cross direction and a small offset to enter
	// the next cell when doing cell crossing.
	vec2 crossStep, crossOffset;
	crossStep.x = (v.x >= 0) ? 1.0 : -1.0;
	crossStep.y = (v.y >= 0) ? 1.0 : -1.0;
	// #todo-ssr: This scaling feels so random. Needs concrete math.
	crossOffset.xy = crossStep * (8.0 / ubo.sceneSize);

	// Set current ray to the original screen coordinate and depth.
	vec3 ray = p;
	float minZ = ray.z;
	float maxZ = ray.z + v.z * maxTraceDistance;
	float deltaZ = maxZ - minZ;

	vec3 o = ray;
	vec3 d = v * maxTraceDistance;

	int level = HIZ_START_LEVEL;
	uint iterations = 0;
	bool isBackwardRay = v.z < 0;
	float rayDir = isBackwardRay ? -1.0 : 1.0;

	// Cross to next cell s.t. we don't get a self-intersection immediately.
	ivec2 startCellCount = getCellCount(level);
	ivec2 rayCell = getCell(ray.xy, startCellCount);
	ray = intersectCellBoundary(o, d, rayCell, startCellCount, crossStep, crossOffset);

	while (level >= HIZ_STOP_LEVEL
			&& ray.z * rayDir <= maxZ * rayDir
			&& iterations < MAX_ITERATIONS)
	{
		// Get the cell number of our current ray.
		ivec2 cellCount = getCellCount(level);
		ivec2 oldCellIx = getCell(ray.xy, cellCount);

		// Get the minimum depth plane in which the current ray resides.
		float cellMinZ = getMinimumDepthPlane(oldCellIx, level);

		// Intersect only if ray depth is below the minimum depth plane.
		vec3 tempRay;
		if (cellMinZ > ray.z && !isBackwardRay) {
			tempRay = intersectDepthPlane(o, d, (cellMinZ - minZ) / deltaZ);
		} else {
			tempRay = ray;
		}

		ivec2 newCellIx = getCell(tempRay.xy, cellCount);

		float thickness = (level == 0) ? (ray.z - cellMinZ) : 0.0;
		bool crossed = (isBackwardRay && (cellMinZ > ray.z))
					|| (thickness > MAX_THICKNESS)
					|| crossedCellBoundary(oldCellIx, newCellIx);
		
		if (crossed) {
			ray = intersectCellBoundary(o, d, oldCellIx, cellCount, crossStep, crossOffset);
			level = min(maxLevel, level + 1);
		} else {
			ray = tempRay;
			level = level - 1;
		}

		iterations += 1;
	}

	// Results
	debugFinalLevel = level;
	debugIterations = iterations;
	hitPointSS = ray;
	return level < HIZ_STOP_LEVEL && iterations < MAX_ITERATIONS;
}

void main() {
	vec2 screenUV = fs_in.screenUV;

	GBufferData gbufferData;
	unpackGBuffer(
		ivec2(screenUV * ubo.sceneSize),
		inGBufferA, inGBufferB, inGBufferC,
		gbufferData);

	float sceneDepth = texture(inSceneDepth, screenUV).x;
	float roughness = gbufferData.roughness;
	
	// Nomenclature
	// SS: Screen space
	// CS: Clip space
	// VS: view space
	// WS: world space

	// View space values
	vec3 positionVS = gbufferData.vs_coords; //getViewPositionFromSceneDepth(screenUV, sceneDepth);
	vec3 normalVS = gbufferData.normal;
	vec3 viewDirectionVS = normalize(positionVS - uboPerFrame.eyePosition);
	vec3 reflectionDirVS = reflect(viewDirectionVS, normalVS);

#if REVERSE_Z
	sceneDepth = 1.0 - sceneDepth;
#endif

	// Screen space values
	vec3 positionSS = vec3(screenUV, sceneDepth);
	vec3 positionCS = positionSS;
	positionCS.xy = 2.0 * positionCS.xy - 1.0;

	vec3 position2VS = positionVS + 100.0 * reflectionDirVS;
	//if (position2VS.z < 0.0) {
	//	position2VS /= position2VS.z;
	//}
	vec4 position2CS = uboPerFrame.projTransform * vec4(position2VS, 1.0);
	position2CS /= position2CS.w;
	vec3 position2SS = position2CS.xyz;
#if REVERSE_Z
	position2SS.z = 1.0 - position2SS.z;
#endif
	position2SS.xy = vec2(0.5) + 0.5 * position2SS.xy;

	vec3 reflectionDirSS = normalize(position2SS - positionSS);

	vec3 hitPointSS;
	int debugFinalLevel;
	uint debugIterations;
	bool intersects = traceHiZ(positionSS, reflectionDirSS, hitPointSS, debugFinalLevel, debugIterations);

	vec3 finalRadiance = vec3(0.0);
	if (intersects) {
		vec3 sceneColorSample = texture(inSceneColor, hitPointSS.xy).xyz;
		finalRadiance = (1.0 - roughness) * sceneColorSample;
	}

	outRayTracingResult = finalRadiance;

#if 0
	// DEBUG: Reflection direction
	//outRayTracingResult = vec3(0.5) + 0.5 * reflectionDirSS;
	// DEBUG: Hit point
	outRayTracingResult = hitPointSS;
	// DEBUG: Num iterations
	//outRayTracingResult = mix(vec3(1,0,0), vec3(0,0,1), float(debugIterations) / float(MAX_ITERATIONS));
	// DEBUG: mip level
	//outRayTracingResult = mix(vec3(1,0,0), vec3(0,0,1), float(debugFinalLevel) / float(ubo.hiZMipCount - 1));
#endif
}
