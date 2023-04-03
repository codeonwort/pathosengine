// Simulates the Earth's atmospheric scattering.

#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;
	class Camera;

	class SkyAtmospherePass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void renderSkyAtmosphere(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);

	private:
		void renderToScreen(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera);
		void renderToCubemap(RenderCommandList& cmdList, SceneProxy* scene);
		void renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene);
		void renderSkyPrefilterMap(RenderCommandList& cmdList, SceneProxy* scene);

		GLuint fbo = 0xffffffff;
		GLuint cubemapTexture = 0;
		UniformBuffer ubo;
		GLuint vao;

	};

}
