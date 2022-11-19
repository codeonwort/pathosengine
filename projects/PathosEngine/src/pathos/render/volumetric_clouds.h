#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/render/render_command_list.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;

	class VolumetricCloudPass : public Noncopyable {

	public:
		~VolumetricCloudPass() = default;

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderVolumetricCloud(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void recreateRenderTarget(RenderCommandList& cmdList, uint32 inWidth, uint32 inHeight, float resolutionScale);

	private:
		uint32 renderTargetWidth = 0;
		uint32 renderTargetHeight = 0;
		UniformBuffer ubo;

		// https://developer.nvidia.com/blog/rendering-in-real-time-with-spatiotemporal-blue-noise-textures-part-1/
		GLuint texSTBN = 0;
	};

}
