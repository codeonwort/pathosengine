#pragma once

#include "pathos/core_minimal.h"
#include "pathos/rhi/uniform_buffer.h"

namespace pathos {

	class SceneProxy;
	class Camera;
	class SphereGeometry;
	class Buffer;

	class VisualizeLightProbePass {

	public:
		VisualizeLightProbePass();
		~VisualizeLightProbePass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		SphereGeometry* sphereGeom = nullptr;
		GLuint fbo = 0;
		UniformBuffer ubo;

	};

}
