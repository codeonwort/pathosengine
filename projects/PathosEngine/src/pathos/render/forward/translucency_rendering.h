#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/scene/camera.h"

namespace pathos {

	class SceneProxy;
	struct StaticMeshProxy;

	class TranslucencyRendering final {
		
	public:
		TranslucencyRendering();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderTranslucency(
			RenderCommandList& cmdList,
			const SceneProxy* scene,
			const Camera* camera);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer uboPerObject;
		UniformBuffer uboLightInfo;

	};

}
