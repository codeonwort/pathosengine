#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;

	class PanoramaSkyPass : public Noncopyable {
		
	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void renderToScreen(RenderCommandList& cmdList, SceneProxy* scene);
		void renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene);
		void renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene);
		void renderSkyPrefilterMap(RenderCommandList& cmdList, SceneProxy* scene);

		GLuint fbo = 0xffffffff;
		GLuint cubemapTexture = 0;
		UniformBuffer ubo;

	};

}
