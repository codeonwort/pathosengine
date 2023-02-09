// Simulates the Earth's atmosphere scattering

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
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;
		GLuint vao;

	};

}
