#include "visualize_sky_occlusion.h"
#include "pathos/rhi/gl_debug_group.h"
#include "pathos/console.h"

// cvars and shaders
namespace pathos {

	static ConsoleVariable<int32> cvar_visSkyOcclusion("r.visualizeSkyOcclusion", 0, "0 = disable, 1 = enable");

	// class VisualizeSkyOcclusionVS

}

namespace pathos {

	VisualizeSkyOcclusionPass::VisualizeSkyOcclusionPass() {}
	VisualizeSkyOcclusionPass::~VisualizeSkyOcclusionPass() {}

	void VisualizeSkyOcclusionPass::initializeResources(RenderCommandList& cmdList) {
		//
	}

	void VisualizeSkyOcclusionPass::releaseResources(RenderCommandList& cmdList) {
		//
	}

	void VisualizeSkyOcclusionPass::renderSkyOcclusion(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) {
		if (cvar_visSkyOcclusion.getInt() == 0) {
			return;
		}

		SCOPED_DRAW_EVENT(VisualizeSkyOcclusion);

		//
	}

}
