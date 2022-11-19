#pragma once

#include "pathos/rhi/render_command_list.h"

// Resolve unlit color into sceneColor.

namespace pathos {

	class MeshGeometry;

	class ResolveUnlitPass {

	public:
		ResolveUnlitPass();
		~ResolveUnlitPass();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderUnlit(
			RenderCommandList& cmdList,
			MeshGeometry* fullscreenQuad);

	private:
		GLuint fbo = 0xffffffff;

		bool destroyed = false;

	};

}
