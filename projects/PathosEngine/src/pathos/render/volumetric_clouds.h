#pragma once

#include "pathos/gl_handles.h"
#include "pathos/render/render_command_list.h"
#include "pathos/shader/uniform_buffer.h"

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

	};

}
