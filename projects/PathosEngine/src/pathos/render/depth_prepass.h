#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/smart_pointer.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class Buffer;
	class Material;
	class SceneProxy;
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

		void renderPreDepth(RenderCommandList& cmdList, SceneProxy* scene, Material* indirectDrawDummyMaterial, LandscapeRendering* landscapeRendering);

	private:
		void reallocateIndirectDrawBuffers(RenderCommandList& cmdList, uint32 maxDrawcalls);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer uboPerObject;

		uniquePtr<Buffer> indirectDrawBuffer;
		uniquePtr<Buffer> modelTransformBuffer;

	};

}
