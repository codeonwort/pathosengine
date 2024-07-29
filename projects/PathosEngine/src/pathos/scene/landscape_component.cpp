#include "landscape_component.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"

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
	float offsetX;
	float offsetY;
	float _pad0;
	float _pad1;
};

namespace pathos {

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

			uint32 baseIndex = 0;
			for (int32 divs = 8; divs > 0; divs /= 2) {
				std::vector<float> tempPositions, tempUVs, tempNormals;
				std::vector<uint32> tempIndices;
				PlaneGeometry::generate(
					sizeX, sizeY, divs, divs,
					PlaneGeometry::Direction::Z, EPrimitiveInitOptions::Default,
					tempPositions, tempUVs, tempNormals, tempIndices);

				positions.insert(positions.end(), tempPositions.begin(), tempPositions.end());
				uvs.insert(uvs.end(), tempUVs.begin(), tempUVs.end());
				normals.insert(normals.end(), tempNormals.begin(), tempNormals.end());

				//for (uint32& ix : tempIndices) ix += baseIndex;
				indices.insert(indices.end(), tempIndices.begin(), tempIndices.end());

				numVertices.push_back((uint32)(tempPositions.size() / 3));
				numIndices.push_back((uint32)tempIndices.size());
				indexOffsets.push_back((int32)baseIndex);

				baseIndex += (uint32)(tempPositions.size() / 3);
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
		
		vector3 cameraPosition = scene->camera.getPosition();
		vector3 cameraDirection = scene->camera.getEyeVector();

		std::vector<DrawElementsIndirectCommand> drawCommands;
		std::vector<LandscapeSectorParameter> sectorParams;
		drawCommands.reserve(countX * countY);
		sectorParams.reserve(countX * countY);

		// #wip-landscape: LOD 0 is OK but LOD 1 looks broken
		const int32 LOD = 1;

		for (int32 sectorX = 0; sectorX < countX; ++sectorX) {
			for (int32 sectorY = 0; sectorY < countY; ++sectorY) {
				DrawElementsIndirectCommand cmd{
					numIndices[LOD],
					1, // instanceCount
					geometry->getFirstIndex(), // CAUTION: Unlike glDrawElements, it's not byte offset.
					indexOffsets[LOD], // baseVertex
					0, // baseInstance
				};
				drawCommands.emplace_back(cmd);

				LandscapeSectorParameter sector{
					vector4((float)sectorX / countX, (float)sectorY / countY, (float)(sectorX + 1) / countX, (float)(sectorY + 1) / countY),
					sectorX * sizeX,
					sectorY * sizeY,
					0.0f,
					0.0f,
				};
				sectorParams.emplace_back(sector);
			}
		}
		
		indirectDrawArgsBuffer->writeToGPU(0, indirectDrawArgsBuffer->getCreateParams().bufferSize, drawCommands.data());
		sectorParameterBuffer->writeToGPU(0, sectorParameterBuffer->getCreateParams().bufferSize, sectorParams.data());

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
