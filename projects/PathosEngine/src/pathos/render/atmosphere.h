// Simulates the Earth's atmosphere scattering

#pragma once

#include "sky.h"
#include "gl_core.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class AtmosphereScattering : public SkyRendering {
		
	public:
		AtmosphereScattering();
		~AtmosphereScattering();

		virtual void render(const Scene* scene, const Camera* camera) override;

	private:
		GLuint program;
		UniformBuffer ubo;
		GLuint vao;

	};

}
