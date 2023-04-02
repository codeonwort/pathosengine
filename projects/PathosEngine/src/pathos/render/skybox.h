#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	class SceneProxy;

	class SkyboxPass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void renderSkyboxToScreen(RenderCommandList& cmdList, SceneProxy* scene);
		void renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene);

		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;

	};

}
