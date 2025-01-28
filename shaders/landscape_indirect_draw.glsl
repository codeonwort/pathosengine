#version 460 core

#include "core/indirect_draw.glsl"

// Fill indirect draw args buffer for landscape indirect draw.

// --------------------------------------------------------
// Definitions

#ifndef BUCKET_SIZE
// Minimum work group size that OpenGL specification guarantees.
//#define BUCKET_SIZE 1024
#error BUCKET_SIZE is not defined.
#endif

struct SectorParameter {
	vec4  uvBounds;
	float offsetX;
	float offsetY;
	uint  lod;
	float _pad0;
};

struct AABB {
	vec3 minBounds;
	vec3 maxBounds;
};

struct Plane3D {
	vec3 normal; // Surface normal
	float distance; // Length of perp vector from O to the plane
};

struct Frustum3D {
	// 0: top, 1: bottom, 2: left, 3: right, 4: near, 5: far
	Plane3D planes[6];
};

// --------------------------------------------------------
// Layout

layout (local_size_x = BUCKET_SIZE) in;

layout (std140, binding = 1) uniform UBO {
	Frustum3D cameraFrustum;
	mat4x4    localToWorld;
	uvec4     indexCountPerLOD; // #todo-landscape: Extend if more LODs are needed.
	uvec4     firstIndexPerLOD;
	vec3      actorPosition;
	float     sectorSizeX;
	vec3      cameraPosition;
	float     sectorSizeY;
	uint      sectorCountX;
	uint      sectorCountY;
	float     cullDistance;
	float     heightMultiplier;
} ubo;

layout (std140, binding = 0) writeonly buffer SSBO_SectorParameter {
	SectorParameter outSectors[];
};

// std430 as the struct is not 16-byte aligned.
layout (std430, binding = 1) writeonly buffer SSBO_DrawArgsBuffer {
	DrawElementsIndirectCommand outCommands[];
};

// --------------------------------------------------------
// Program

uvec2 getSectorCoord(uint linearIx) {
	return uvec2(linearIx % ubo.sectorCountX, linearIx / ubo.sectorCountX);
}

vec4 getSectorUVBounds(uvec2 sectorCoord) {
	float x = float(sectorCoord.x);
	float y = float(sectorCoord.y);
	float cx = float(ubo.sectorCountX);
	float cy = float(ubo.sectorCountY);
	return vec4(x / cx, y / cy, (x + 1.0) / cx, (y + 1.0) / cy);
}

#define FLT_MAX (3.402823466e+38F)

AABB calculateWorldBounds(vec3 localMin, vec3 localMax, mat4x4 localToWorld) {
	vec3 vs[8];
	for (uint i = 0; i < 8; ++i) {
		vs[i].x = bool(i & 1) ? localMin.x : localMax.x;
		vs[i].y = bool((i >> 1) & 1) ? localMin.y : localMax.y;
		vs[i].z = bool((i >> 2) & 1) ? localMin.z : localMax.z;
	}
	vec3 worldMin = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	vec3 worldMax = -vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	for (uint i = 0; i < 8; ++i) {
		vs[i] = (localToWorld * vec4(vs[i], 1.0)).xyz;
		worldMin = min(worldMin, vs[i]);
		worldMax = max(worldMax, vs[i]);
	}
	return AABB(worldMin, worldMax);
}

// true if AABB is on positive side of the plane.
bool hitTest_AABB_plane(AABB box, Plane3D plane) {
	vec3 boxCenter = 0.5 * (box.maxBounds + box.minBounds);
	vec3 boxHalfSize = 0.5 * (box.maxBounds - box.minBounds);
	float r = dot(boxHalfSize, abs(plane.normal));
	float s = dot(boxCenter, plane.normal) - plane.distance;
	return -r <= s;
}
// true if AABB is inside of the frustum.
bool hitTest_AABB_frustum(AABB box, Frustum3D frustum) {
	for (int i = 0; i < 6; ++i) {
		if (!hitTest_AABB_plane(box, frustum.planes[i])) {
			return false;
		}
	}
	return true;
}
// Same as hitTest_AABB_frustum(), but ignore the far plane.
bool hitTest_AABB_frustumNoFarPlane(AABB box, Frustum3D frustum) {
	for (int i = 0; i < 5; ++i) {
		if (!hitTest_AABB_plane(box, frustum.planes[i])) {
			return false;
		}
	}
	return true;
}

void main() {
	uint ix = gl_GlobalInvocationID.x;
	if (ix >= ubo.sectorCountX * ubo.sectorCountY) return;

	uvec2 sectorCoord = getSectorCoord(ix);
	vec2 fSectorCoord = vec2(sectorCoord);
	vec2 sectorSize = vec2(ubo.sectorSizeX, ubo.sectorSizeY);
	vec4 uvBounds = getSectorUVBounds(sectorCoord);

	// In a game world, +Y is upward. We ignore y and consider xz as xy in this shader.
	vec2 basePositionXY = ubo.actorPosition.xz;
	vec2 cameraXY = ubo.cameraPosition.xz;

	vec2 sectorCenter = basePositionXY;
	sectorCenter += vec2(sectorSize.x, -sectorSize.y) * vec2(fSectorCoord.x + 0.5, fSectorCoord.y + 0.5);
	vec2 unitOffset = abs((sectorCenter - cameraXY) / sectorSize);
	uint distanceToCamera = uint(max(unitOffset.x, unitOffset.y));

	uint LOD_FACTOR = 3;
	uint LOD = min(distanceToCamera / LOD_FACTOR, 2);

	// Frustum culling
	bool bFrustumCulled = false;
	{
		vec3 minV = vec3(sectorSize.x * fSectorCoord.x, sectorSize.y * fSectorCoord.y, 0.0);
		vec3 maxV = minV + vec3(sectorSize.x, sectorSize.y, ubo.heightMultiplier);
		mat4x4 localToWorld = ubo.localToWorld;
		AABB worldBounds = calculateWorldBounds(minV, maxV, localToWorld);
		bool bInFrustum = hitTest_AABB_frustumNoFarPlane(worldBounds, ubo.cameraFrustum);

		bFrustumCulled = !bInFrustum;
	}

	// Distance culling
	bool bDistanceCulled = distanceToCamera >= ubo.cullDistance;

	bool bCulled = bFrustumCulled || bDistanceCulled;

	// Output
	outSectors[ix] = SectorParameter(
		uvBounds,                      // uvBounds
		sectorSize.x * fSectorCoord.x, // offsetX
		sectorSize.y * fSectorCoord.y, // offsetY
		LOD,                           // lod
		0.0                            // _pad0
	);
	outCommands[ix] = DrawElementsIndirectCommand(
		ubo.indexCountPerLOD[LOD], // count
		bCulled ? 0 : 1,           // instanceCount
		ubo.firstIndexPerLOD[LOD], // firstIndex
		0,                         // baseVertex
		0                          // baseInstance
	);
}
