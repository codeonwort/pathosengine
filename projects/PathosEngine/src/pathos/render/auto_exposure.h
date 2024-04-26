#pragma once

#include "pathos/rhi/render_command_list.h"

namespace pathos {

	class SceneProxy;
	class MeshGeometry;

	// Calculate average brightness for auto exposure control.
	class AutoExposurePass {

	public:
		AutoExposurePass();
		~AutoExposurePass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderAutoExposure(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		GLuint fbo = 0xffffffff;
		MeshGeometry* fullscreenQuad = nullptr;

		bool destroyed = false;

	};

}
