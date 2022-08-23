#pragma once

#include "pathos/render/render_command_list.h"
#include "pathos/shader/uniform_buffer.h"

// - Calculate indirect lighting (global illumination) and write to sceneColor.
// - Direct lighting pass already wrote its result to sceneColor,
//   so we use color blending (ADD) to combine their results.

namespace pathos {

	class Camera;
	class SceneProxy;
	class MeshGeometry;

	class IndirectLightingPass {

	public:
		IndirectLightingPass();
		~IndirectLightingPass();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void renderIndirectLighting(
			RenderCommandList& cmdList,
			SceneProxy* scene,
			Camera* camera,
			MeshGeometry* fullscreenQuad);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;

		bool destroyed = false;

	};

}