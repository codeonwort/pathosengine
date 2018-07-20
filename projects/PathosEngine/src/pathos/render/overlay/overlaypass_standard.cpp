#include "overlaypass_standard.h"
#include "pathos/shader/shader.h"

namespace pathos {

	static constexpr unsigned int TEXTURE_UNIT = 0;

	OverlayPass_Standard::OverlayPass_Standard() {
		createProgram();
	}

	OverlayPass_Standard::~OverlayPass_Standard() {
		glDeleteProgram(program);
	}

	void OverlayPass_Standard::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("overlay_standard_vs.glsl");
		fs.loadSource("overlay_standard_fs.glsl");
		program = pathos::createProgram(vs, fs);

#define GETUNIFORM(uname) { uniform_##uname = glGetUniformLocation(program, #uname); assert(uniform_##uname != -1); }
		GETUNIFORM(transform);
		GETUNIFORM(color);
#undef GETUNIFORM
	}

	void OverlayPass_Standard::render(DisplayObject2D* object, const Transform& transformAccum) {
		glUseProgram(program);

		MeshGeometry* geom = object->getGeometry();
		glm::mat4 transform = transformAccum.getMatrix();

		// uniform
		glUniformMatrix4fv(uniform_transform, 1, false, &transform[0][0]);
		glUniform4fv(uniform_color, 1, rgba);
		//glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT);
		//glBindTexture(GL_TEXTURE_2D, 0);

		geom->activate_position_uv();
		geom->activateIndexBuffer();
		geom->draw();
		geom->deactivate();
		geom->deactivateIndexBuffer();
	}

	void OverlayPass_Standard::setUniform_color(float newRGBA[4]) {
		memcpy(rgba, newRGBA, sizeof(float) * 4);
	}

}