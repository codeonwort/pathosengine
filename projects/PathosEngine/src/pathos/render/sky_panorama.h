#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;
	class Texture;

	class PanoramaSkyPass : public Noncopyable {
		
	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderPanoramaSky(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void renderToScreen(RenderCommandList& cmdList, SceneProxy* scene);
		void renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene, int32 faceBegin, int32 faceEnd); // faces inclusive
		void generateCubemapMips(RenderCommandList& cmdList);
		void computeDiffuseSH(RenderCommandList& cmdList);
		void filterSpecular(RenderCommandList& cmdList);

		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;
		Texture* reflectionCubemap = nullptr;
		Texture* ambientCubemap = nullptr;

	};

}
