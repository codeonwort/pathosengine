#pragma once

#include "badger/types/noncopyable.h"

#include "render_command_list.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class SceneProxy;
	class Camera;

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

		void renderPreDepth(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer uboPerObject;

	};

}
