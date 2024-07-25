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
		geometries.clear();
	}

	void LandscapeComponent::initializeSectors(float inSizeX, float inSizeY, int32 inCountX, int32 inCountY) {
		sizeX = inSizeX; sizeY = inSizeY; countX = inCountX; countY = inCountY;

		for (int32 divs = 8; divs > 0; divs /= 2) {
			MeshGeometry* G = new PlaneGeometry(sizeX, sizeY, divs, divs);
			geometries.push_back(uniquePtr<MeshGeometry>(G));
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
		bool bValid = sizeX > 0.0f && sizeY > 0.0f && countX > 0 && countY > 0
			&& geometries.size() > 0 && material != nullptr && indirectDrawArgsBuffer != nullptr;
		if (!bValid) {
			return;
		}
		
		vector3 cameraPosition = scene->camera.getPosition();
		vector3 cameraDirection = scene->camera.getEyeVector();

		std::vector<DrawElementsIndirectCommand> drawCommands;
		std::vector<LandscapeSectorParameter> sectorParams;
		drawCommands.reserve(countX * countY);
		sectorParams.reserve(countX * countY);

		// #wip-landscape: Calculate proper LOD per sector
		//const int32 LOD = (sectorY * countX + sectorX) % geometries.size();
		const int32 LOD = 0;
		MeshGeometry* G = geometries[LOD].get();

		for (int32 sectorX = 0; sectorX < countX; ++sectorX) {
			for (int32 sectorY = 0; sectorY < countY; ++sectorY) {				
				DrawElementsIndirectCommand cmd{
					G->getIndexCount(),
					1, // instanceCount
					G->getFirstIndex(), // CAUTION: Unlike glDrawElements, it's not byte offset.
					0, // baseVertex
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
		proxy->tempGeometry           = G;
		proxy->material               = material;
		proxy->indirectDrawCount      = countX * countY;
		proxy->modelMatrix            = getLocalMatrix();
		proxy->prevModelMatrix        = prevModelMatrix;

		scene->addLandscapeProxy(proxy);

		prevModelMatrix = getLocalMatrix();
	}

}
