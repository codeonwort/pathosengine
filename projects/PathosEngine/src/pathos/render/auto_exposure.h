#pragma once

#include "pathos/rhi/render_command_list.h"
#include "pathos/rhi/uniform_buffer.h"

namespace pathos {

	class SceneProxy;
	class MeshGeometry;
	class Buffer;

	// Calculate average brightness of scene for auto exposure control.
	class AutoExposurePass {

	public:
		AutoExposurePass();
		~AutoExposurePass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderAutoExposure(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void renderAutoExposure_averageLogLuminance(RenderCommandList& cmdList, SceneProxy* scene);
		void renderAutoExposure_luminanceHistogram(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		bool bDestroyed = false;

		GLuint fbo = 0xffffffff;
		MeshGeometry* fullscreenQuad = nullptr;

		Buffer* luminanceHistogram = nullptr;
		UniformBuffer uboHistogramGen;
		UniformBuffer uboHistogramAvg;
	};

}
