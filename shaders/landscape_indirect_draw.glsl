#version 460 core

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

struct DrawElementsIndirectCommand {
	uint count;
	uint instanceCount;
	uint firstIndex;
	int  baseVertex;
	uint baseInstance;
};

// --------------------------------------------------------
// Layout

layout (local_size_x = BUCKET_SIZE) in;

layout (std140, binding = 1) uniform UBO {
	uvec4 indexCountPerLOD; // #todo-landscape: Extend if more LODs are needed.
	uvec4 firstIndexPerLOD;
	vec3  actorPosition;
	float sectorSizeX;
	vec3  cameraPosition;
	float sectorSizeY;
	uint  sectorCountX;
	uint  sectorCountY;
	float _pad0;
	float _pad1;
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
	sectorCenter += sectorSize * vec2(fSectorCoord.x + 0.5, fSectorCoord.y + 0.5);
	vec2 unitOffset = abs((sectorCenter - cameraXY) / sectorSize);
	uint distanceToCamera = uint(max(unitOffset.x, unitOffset.y));

	uint LOD = min(distanceToCamera, 2);

	// #wip-landscape: Perform culling
	bool bCulled = false;

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
