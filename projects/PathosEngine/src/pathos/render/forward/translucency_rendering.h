#pragma once

#include "pathos/render/renderer.h"
#include "pathos/render/render_command_list.h"
#include "pathos/shader/uniform_buffer.h"
#include "pathos/camera/camera.h"

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
