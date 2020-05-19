#pragma once

#include "pathos/render/renderer.h"
#include "pathos/render/render_command_list.h"
#include "pathos/shader/uniform_buffer.h"
#include "pathos/camera/camera.h"

namespace pathos {

	struct StaticMeshProxy;

	class TranslucencyRendering final {
		
	public:
		TranslucencyRendering();
		~TranslucencyRendering();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderTranslucency(RenderCommandList& cmdList, const Camera* camera, const std::vector<StaticMeshProxy*>& meshBatches);

	private:
		GLuint fbo = 0xffffffff;
		GLuint shaderProgram = 0;
		UniformBuffer ubo;

	};

}
