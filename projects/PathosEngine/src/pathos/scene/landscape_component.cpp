#include "landscape_component.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"

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

	void LandscapeComponent::createRenderProxy(SceneProxy* scene) {
		const bool bValid = sizeX > 0.0f && sizeY > 0.0f && countX > 0 && countY > 0
			&& geometry != nullptr && material != nullptr && indirectDrawArgsBuffer != nullptr;
		if (!bValid) {
			return;
		}
		
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

		material->setConstantParameter("baseDivisions", LANDSCAPE_BASE_DIVISIONS);

		LandscapeProxy* proxy = ALLOC_RENDER_PROXY<LandscapeProxy>(scene);

		proxy->indirectDrawArgsBuffer = indirectDrawArgsBuffer.get();
		proxy->sectorParameterBuffer  = sectorParameterBuffer.get();
		proxy->geometry               = geometry.get();
		proxy->material               = material;
		proxy->indirectDrawCount      = countX * countY;
		proxy->modelMatrix            = getLocalMatrix();
		proxy->prevModelMatrix        = prevModelMatrix;

		scene->addLandscapeProxy(proxy);

		prevModelMatrix = getLocalMatrix();
	}

}
