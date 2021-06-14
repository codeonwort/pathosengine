// Simulates the Earth's atmosphere scattering

#pragma once

#include "sky.h"
#include "gl_core.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class AtmosphereScattering : public SkyActor {
		
	public:
		virtual void render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) override;

	public:
		virtual void onSpawn() override;
		virtual void onDestroy() override;

	private:
		UniformBuffer ubo;
		GLuint vao;

	};

}
