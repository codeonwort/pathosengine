#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;
	class Camera;
	class LandscapeRendering;

	// Render only scene depth without any shading.
	//
	// The result can be utilized in various situations:
	// - Generate HiZ for hardware occlusion.
	// - Depth-stencil test to avoid shading of occluded geometries in the base pass.
	// - Early-exit for raymarching or post-processing passes.
	class DepthPrepass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderPreDepth(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera, LandscapeRendering* landscapeRendering);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer uboPerObject;

	};

}
