#pragma once

#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"

// - Calculate indirect lighting (global illumination) and write to sceneColor.
// - Direct lighting pass already wrote its result to sceneColor,
//   so we use color blending (ADD) to combine their results.

namespace pathos {

	class SceneProxy;
	class MeshGeometry;
	class Buffer;
	template<typename> class ConsoleVariable;

	extern ConsoleVariable<int32> cvar_indirectLighting;

	class IndirectLightingPass {

	public:
		IndirectLightingPass();
		~IndirectLightingPass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderIndirectLighting(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void renderIndirectDiffuse(RenderCommandList& cmdList, SceneProxy* scene);
		void renderIndirectSpecular(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer uboDiffuse;
		UniformBuffer uboSpecular;

		bool destroyed = false;

	};

}
