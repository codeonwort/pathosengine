#include "landscape_component.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/util/image_data.h"

#include "badger/types/half_float.h"
#include "badger/math/minmax.h"

// #wip-indirect-draw: Fill draw buffer from GPU
#define LANDSCAPE_GPU_DRIVEN 1

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMultiDrawElementsIndirect.xhtml
struct DrawElementsIndirectCommand {
	uint32  count;
	uint32  instanceCount;
	uint32  firstIndex;
	int32   baseVertex;
	uint32  baseInstance;
};

struct LandscapeSectorParameter {
	vector4 uvBounds;
	float   offsetX;
	float   offsetY;
	uint32  lod;
	float   _pad0;
};

namespace pathos {

	static const int32 LANDSCAPE_BASE_DIVISIONS = 16;

	LandscapeComponent::LandscapeComponent() {
	}

	LandscapeComponent::~LandscapeComponent() {
		indirectDrawArgsBuffer.reset();
		sectorParameterBuffer.reset();
		geometry.reset();
	}

	void LandscapeComponent::initializeSectors(float inSizeX, float inSizeY, int32 inCountX, int32 inCountY) {
		sizeX = inSizeX; sizeY = inSizeY; countX = inCountX; countY = inCountY;

		// Combine all LODs into a single mesh.
		{
			std::vector<float> positions, uvs, normals;
			std::vector<uint32> indices;

			uint32 baseVertex = 0;
			uint32 baseIndex = 0;

			for (int32 divs = LANDSCAPE_BASE_DIVISIONS; divs > 0; divs /= 2) {
				std::vector<float> tempPositions, tempUVs, tempNormals;
				std::vector<uint32> tempIndices;
				PlaneGeometry::generate(
					sizeX, sizeY, divs, divs,
					PlaneGeometry::Direction::Z, EPrimitiveInitOptions::Default,
					tempPositions, tempUVs, tempNormals, tempIndices);

				positions.insert(positions.end(), tempPositions.begin(), tempPositions.end());
				uvs.insert(uvs.end(), tempUVs.begin(), tempUVs.end());
				normals.insert(normals.end(), tempNormals.begin(), tempNormals.end());

				for (uint32& ix : tempIndices) ix += baseVertex;
				indices.insert(indices.end(), tempIndices.begin(), tempIndices.end());

				numVertices.push_back((uint32)(tempPositions.size() / 3));
				numIndices.push_back((uint32)tempIndices.size());
				indexOffsets.push_back(baseIndex);

				baseVertex += (uint32)(tempPositions.size() / 3);
				baseIndex += (uint32)tempIndices.size();
			}

			geometry = makeUnique<MeshGeometry>();
			geometry->initializeVertexLayout(MeshGeometry::EVertexAttributes::All);
			geometry->updatePositionData(positions.data(), (uint32)positions.size());
			geometry->updateUVData(uvs.data(), (uint32)uvs.size());
			geometry->updateNormalData(normals.data(), (uint32)normals.size());
			geometry->updateIndexData(indices.data(), (uint32)indices.size());
			geometry->calculateTangentBasis();
		}

		{
			BufferCreateParams bufferCreateParams{
				EBufferUsage::CpuWrite,
				sizeof(DrawElementsIndirectCommand) * countX * countY,
				nullptr, // initialData
				"Buffer_Landscape_IndirectDrawArgs",
			};
			indirectDrawArgsBuffer.reset();
			indirectDrawArgsBuffer = makeUnique<Buffer>(bufferCreateParams);
			indirectDrawArgsBuffer->createGPUResource();
		}
		{
			BufferCreateParams bufferCreateParams{
				EBufferUsage::CpuWrite,
				sizeof(LandscapeSectorParameter) * countX * countY,
				nullptr, // initialData
				"Buffer_Landscape_SectorParameter",
			};
			sectorParameterBuffer.reset();
			sectorParameterBuffer = makeUnique<Buffer>(bufferCreateParams);
			sectorParameterBuffer->createGPUResource();
		}
	}

	void LandscapeComponent::initializeHeightMap(ImageBlob* blob) {
		CHECK(blob->glStorageFormat == GL_R8 || blob->glStorageFormat == GL_R16);
		const uint32 stride = (blob->glStorageFormat == GL_R8) ? 1 : 2;

		uint8* streamU8 = (uint8*)blob->rawBytes;
		uint16* streamU16 = (uint16*)blob->rawBytes;

		heightMapWidth = blob->width;
		heightMapHeight = blob->height;
		heightMapValues.reserve(heightMapWidth * heightMapHeight);
		for (uint32 y = 0; y < heightMapHeight; ++y) {
			for (uint32 x = 0; x < heightMapWidth; ++x) {
				// #wip-heightmap: Should I flip Y?
				uint32 linearIx = y * heightMapWidth + x;
				//uint32 linearIx = (heightMapHeight - y - 1) * heightMapWidth + x;
				//float h = (stride == 1) ? ((float)streamU8[linearIx] / 255.0f) : half_to_float(streamU16[linearIx]);
				float h = (stride == 1) ? ((float)streamU8[linearIx] / 255.0f) : ((float)streamU16[linearIx] / 65535.0f);
				heightMapValues.push_back(h);
			}
		}
	}

	vector2 LandscapeComponent::getNormalizedUV(float x, float z) const {
		float u = badger::clamp(0.0f, x / (sizeX * (float)countX), 1.0f);
		float v = badger::clamp(0.0f, z / (sizeY * (float)countY), 1.0f);
		return vector2(u, v);
	}

	float LandscapeComponent::sampleHeightmap(float u, float v) const {
		if (heightMapValues.size() == 0) return 0.0f;
		u = badger::clamp(0.0f, u, 1.0f);
		v = badger::clamp(0.0f, v, 1.0f);
		float du = 1.0f / (float)heightMapWidth;
		float dv = 1.0f / (float)heightMapHeight;

		int32 maxX = (int32)heightMapWidth - 1;
		int32 maxY = (int32)heightMapHeight - 1;

		int32 x1 = badger::min(int32(u * (float)heightMapWidth), maxX - 1);
		int32 y1 = badger::min(int32(v * (float)heightMapWidth), maxY - 1);
		int32 x2 = badger::min(x1 + 1, maxX);
		int32 y2 = badger::min(y1 + 1, maxY);

		float u1 = (float)x1 / heightMapWidth;
		float v1 = (float)y1 / heightMapHeight;
		float u2 = u1 + du;
		float v2 = v1 + dv;

		float ku = (u2 - u) / du;
		float kv = (v2 - v) / dv;

		// #wip-heightmap: Bilinear interpolation is not the way?
		float h00 = heightMapValues[y1 * (int32)heightMapWidth + x1];
		float h10 = heightMapValues[y1 * (int32)heightMapWidth + x2];
		float h01 = heightMapValues[y2 * (int32)heightMapWidth + x1];
		float h11 = heightMapValues[y2 * (int32)heightMapWidth + x2];
		float h0 = h00 + ku * (h10 - h00);
		float h1 = h01 + ku * (h11 - h01);
		float h = h0 + kv * (h1 - h0);

		return h;
	}

	void LandscapeComponent::createRenderProxy(SceneProxy* scene) {
		const bool bValid = sizeX > 0.0f && sizeY > 0.0f && countX > 0 && countY > 0
			&& geometry != nullptr && material != nullptr
			&& sectorParameterBuffer != nullptr && indirectDrawArgsBuffer != nullptr;
		if (!bValid) {
			return;
		}

		// #wip-indirect-draw: If gpu driven, gbuffer_pass will run a landscape culling shader and fill the buffers.
		if (LANDSCAPE_GPU_DRIVEN == 0) {
			fillIndirectDrawBuffers(scene);
		}
		
		const vector3 cameraPosition = scene->camera.getPosition();
		const vector2 cameraXY = vector2(cameraPosition.x, cameraPosition.z);
		const vector3 cameraDirection = scene->camera.getEyeVector();

		material->setConstantParameter("baseDivisions", LANDSCAPE_BASE_DIVISIONS);

		LandscapeProxy* proxy = ALLOC_RENDER_PROXY<LandscapeProxy>(scene);

		proxy->indirectDrawArgsBuffer  = indirectDrawArgsBuffer.get();
		proxy->sectorParameterBuffer   = sectorParameterBuffer.get();
		proxy->geometry                = geometry.get();
		proxy->material                = material;
		proxy->modelMatrix             = getLocalMatrix();
		proxy->prevModelMatrix         = prevModelMatrix;
		proxy->indirectDrawCount       = countX * countY;
		proxy->bGpuDriven              = (bool)LANDSCAPE_GPU_DRIVEN;
		// For gpu driven
		CHECK(numIndices.size() >= 4 && indexOffsets.size() >= 4);
		for (uint32 i = 0; i < 4; ++i) {
			proxy->indexCountPerLOD[i] = numIndices[i];
			proxy->firstIndexPerLOD[i] = geometry->getFirstIndex() + indexOffsets[i];
		}
		proxy->actorPosition           = getLocation();
		proxy->sectorSizeX             = sizeX;
		proxy->cameraPosition          = cameraPosition;
		proxy->sectorSizeY             = sizeY;
		proxy->sectorCountX            = countX;
		proxy->sectorCountY            = countY;

		scene->addLandscapeProxy(proxy);

		prevModelMatrix = getLocalMatrix();
	}

	void LandscapeComponent::fillIndirectDrawBuffers(SceneProxy* scene) {
		const vector3 cameraPosition = scene->camera.getPosition();
		const vector2 cameraXY = vector2(cameraPosition.x, cameraPosition.z);
		const vector3 cameraDirection = scene->camera.getEyeVector();

		std::vector<DrawElementsIndirectCommand> drawCommands;
		std::vector<LandscapeSectorParameter> sectorParams;
		drawCommands.reserve(countX * countY);
		sectorParams.reserve(countX * countY);

		const vector3 basePosition = getLocation();
		const vector2 basePositionXY = vector2(basePosition.x, basePosition.z);

		for (int32 sectorY = 0; sectorY < countY; ++sectorY) {
			for (int32 sectorX = 0; sectorX < countX; ++sectorX) {
				// Calculate LOD (criteria: distance to camera).
				vector2 sectorCenterXY = basePositionXY;
				sectorCenterXY += vector2(((float)sectorX + 0.5f) * sizeX, ((float)sectorY + 0.5f) * sizeY);
				vector2 unitOffset = glm::abs((sectorCenterXY - cameraXY) / vector2(sizeX, sizeY));
				uint32 distanceToCamera = (uint32)(std::max(unitOffset.x, unitOffset.y));

				const int32 LOD = std::min(distanceToCamera, 2u);

				DrawElementsIndirectCommand cmd{
					numIndices[LOD],
					1, // instanceCount
					geometry->getFirstIndex() + indexOffsets[LOD], // CAUTION: Unlike glDrawElements, it's not byte offset.
					0, // baseVertex
					0, // baseInstance
				};
				drawCommands.emplace_back(cmd);

				LandscapeSectorParameter sector{
					vector4((float)sectorX / countX, (float)sectorY / countY, (float)(sectorX + 1) / countX, (float)(sectorY + 1) / countY),
					sectorX * sizeX,
					sectorY * sizeY,
					(uint32)LOD,
					0.0f,
				};
				sectorParams.emplace_back(sector);
			}
		}

		indirectDrawArgsBuffer->writeToGPU(0, indirectDrawArgsBuffer->getCreateParams().bufferSize, drawCommands.data());
		sectorParameterBuffer->writeToGPU(0, sectorParameterBuffer->getCreateParams().bufferSize, sectorParams.data());
	}

}
