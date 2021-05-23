#include "atmosphere.h"
#include "pathos/shader/shader.h"

namespace pathos {

	struct UBO_Atmosphere {
		glm::vec4 sunParams;     // (sunSizeMultiplier, sunIntensity)
	};

	void AtmosphereScattering::render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera)
	{
		SCOPED_DRAW_EVENT(AtmosphereScattering);

		cmdList.useProgram(program);

		cmdList.depthFunc(GL_GEQUAL);

		UBO_Atmosphere uboData;
		uboData.sunParams.x = 5.0f;
		uboData.sunParams.y = 13.61839144264511f;
		ubo.update(cmdList, 1, &uboData);

		cmdList.bindVertexArray(vao);
		cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
		cmdList.bindVertexArray(0);
	}

	void AtmosphereScattering::onSpawn() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("fullscreen_quad.glsl");
		fs.loadSource("atmosphere.glsl");

		program = pathos::createProgram(vs, fs, "AtmosphereScattering");
		ubo.init<UBO_Atmosphere>();

		glGenVertexArrays(1, &vao);
	}

	void AtmosphereScattering::onDestroy() {
		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
	}

}
