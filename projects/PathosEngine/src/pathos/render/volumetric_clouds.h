#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;
	class Sampler;

	class VolumetricCloudPass : public Noncopyable {

	public:
		~VolumetricCloudPass() = default;

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		// Render volumetric clouds to a separate texture.
		void renderVolumetricCloud(RenderCommandList& cmdList, SceneProxy* scene);

		// Blend volumetric clouds with sceneColor.
		void renderVolumetricCloudPost(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void recreateRenderTarget(RenderCommandList& cmdList, uint32 inWidth, uint32 inHeight, float resolutionScale);

		bool isPassEnabled(const SceneProxy* scene) const;

	private:
		uint32 renderTargetWidth = 0;
		uint32 renderTargetHeight = 0;
		UniformBuffer ubo;

		Sampler* cloudNoiseSampler = nullptr;

		GLuint fboPost = 0xffffffff;

		// https://developer.nvidia.com/blog/rendering-in-real-time-with-spatiotemporal-blue-noise-textures-part-1/
		GLuint texSTBN = 0;
		bool bHasValidResources = false;
	};

}
