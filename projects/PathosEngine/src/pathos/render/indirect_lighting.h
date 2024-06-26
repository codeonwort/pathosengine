#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"

// - Calculate indirect lighting (global illumination) and write to sceneColor.
// - Direct lighting pass already wrote its result to sceneColor,
//   so we use color blending (ADD) to combine their results.

namespace pathos {

	class Camera;
	class SceneProxy;
	class MeshGeometry;
	class Buffer;

	class IndirectLightingPass {

	public:
		IndirectLightingPass();
		~IndirectLightingPass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderIndirectLighting(
			RenderCommandList& cmdList,
			SceneProxy* scene,
			Camera* camera,
			MeshGeometry* fullscreenQuad);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;
		Buffer* ssbo0 = nullptr; // For irradiance volume
		Buffer* ssbo1 = nullptr; // For reflection probe

		bool destroyed = false;

	};

}
