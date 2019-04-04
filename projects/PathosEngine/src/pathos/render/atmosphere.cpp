#include "atmosphere.h"
#include "pathos/shader/shader.h"

namespace pathos {

	struct UBO_Atmosphere {
		glm::vec4 sunParams;     // (sunSizeMultiplier, sunIntensity)
	};

	AtmosphereScattering::AtmosphereScattering()
	{
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("fullscreen_quad.glsl");
		fs.loadSource("atmosphere.glsl");

		program = pathos::createProgram(vs, fs);
		ubo.init<UBO_Atmosphere>();

		glGenVertexArrays(1, &vao);
	}

	AtmosphereScattering::~AtmosphereScattering()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

	void AtmosphereScattering::render(const Scene* scene, const Camera* camera)
	{
		SCOPED_DRAW_EVENT(AtmosphereScattering);

		glUseProgram(program);

		glDepthFunc(GL_LEQUAL);

		UBO_Atmosphere uboData;
		uboData.sunParams.x = 5.0f;
		uboData.sunParams.y = 13.61839144264511f;
		ubo.update(1, &uboData);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);
	}

}
