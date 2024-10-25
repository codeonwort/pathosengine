#include "landscape_rendering.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/buffer.h"
#include "pathos/scene/landscape_component.h"

#include "badger/math/plane.h"

namespace pathos {

	struct UBO_LandscapeCulling {
		static constexpr uint32 BINDING_POINT = 1;

		Frustum3D cameraFrustum;
		matrix4   localToWorld;
		vector4ui indexCountPerLOD;
		vector4ui firstIndexPerLOD;
		vector3   actorPosition;
		float     sectorSizeX;
		vector3   cameraPosition;
		float     sectorSizeY;
		uint32    sectorCountX;
		uint32    sectorCountY;
		float     cullDistance;
		float     heightMultiplier;
	};

	class LandscapeCullingCS : public ShaderStage {
	public:
		static const uint32 BUCKET_SIZE = 1024u;
		LandscapeCullingCS() : ShaderStage(GL_COMPUTE_SHADER, "LandscapeCullingCS") {
			addDefine("BUCKET_SIZE", BUCKET_SIZE);
			setFilepath("landscape_indirect_draw.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_LandscapeCulling, LandscapeCullingCS);

}

namespace pathos {

	LandscapeRendering::LandscapeRendering() {}
	LandscapeRendering::~LandscapeRendering() {}

	void LandscapeRendering::initializeResources(RenderCommandList& cmdList) {
		uboLandscapeCulling.init<UBO_LandscapeCulling>("UBO_LandscapeCulling");
	}

	void LandscapeRendering::releaseResources(RenderCommandList& cmdList) {
		uboLandscapeCulling.safeDestroy();
	}

	void LandscapeRendering::preprocess(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) {
		const std::vector<LandscapeProxy*>& proxyList = scene->getLandscapeMeshes();
		const size_t numProxies = proxyList.size();
		bool bPrepareGpuDriven = false;
		for (size_t proxyIx = 0; proxyIx < numProxies && !bPrepareGpuDriven; ++proxyIx) {
			bPrepareGpuDriven = bPrepareGpuDriven || proxyList[proxyIx]->bGpuDriven;
		}

		if (bPrepareGpuDriven) {
			SCOPED_DRAW_EVENT(LandscapeGpuCulling);

			ShaderProgram& cullingProgram = FIND_SHADER_PROGRAM(Program_LandscapeCulling);
			cmdList.useProgram(cullingProgram.getGLName());

			Frustum3D cameraFrustum;
			camera->getFrustumPlanes(cameraFrustum);

			for (size_t proxyIx = 0; proxyIx < numProxies; ++proxyIx) {
				LandscapeProxy* proxy = proxyList[proxyIx];
				if (proxy->bGpuDriven == false) continue;

				UBO_LandscapeCulling uboData;
				uboData.cameraFrustum = cameraFrustum;
				uboData.localToWorld = proxy->modelMatrix;
				uboData.indexCountPerLOD = proxy->indexCountPerLOD;
				uboData.firstIndexPerLOD = proxy->firstIndexPerLOD;
				uboData.actorPosition = proxy->actorPosition;
				uboData.sectorSizeX = proxy->sectorSizeX;
				uboData.cameraPosition = camera->getPosition();
				uboData.sectorSizeY = proxy->sectorSizeY;
				uboData.sectorCountX = proxy->sectorCountX;
				uboData.sectorCountY = proxy->sectorCountY;
				uboData.cullDistance = proxy->cullDistance;
				uboData.heightMultiplier = proxy->heightMultiplier;

				uboLandscapeCulling.update(cmdList, UBO_LandscapeCulling::BINDING_POINT, &uboData);

				proxy->sectorParameterBuffer->bindAsSSBO(cmdList, 0);
				proxy->indirectDrawArgsBuffer->bindAsSSBO(cmdList, 1);

				uint32 groupSize = (proxy->indirectDrawCount + LandscapeCullingCS::BUCKET_SIZE - 1) / LandscapeCullingCS::BUCKET_SIZE;
				cmdList.dispatchCompute(groupSize, 1, 1);
			}
		}
	}

}
