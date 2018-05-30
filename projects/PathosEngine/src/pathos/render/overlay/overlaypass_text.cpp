#include "overlaypass_text.h"
#include "pathos/overlay/label.h"
#include "pathos/render/shader.h"

namespace pathos {

	static constexpr unsigned int TEXTURE_UNIT = 0;

	OverlayPass_Text::OverlayPass_Text() {
		createProgram();
	}

	OverlayPass_Text::~OverlayPass_Text() {
		glDeleteProgram(program);
	}

	void OverlayPass_Text::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("overlay_text_vs.glsl");
		fs.loadSource("overlay_text_fs.glsl");
		program = pathos::createProgram(vs, fs);

#define GETUNIFORM(uname) { uniform_##uname = glGetUniformLocation(program, #uname); assert(uniform_##uname != -1); }
		GETUNIFORM(transform);
		GETUNIFORM(color);
#undef GETUNIFORM
	}

	void OverlayPass_Text::render(DisplayObject2D* object, const Transform& transformAccum) {
		Label* label = static_cast<Label*>(object);

		glUseProgram(program);

		MeshGeometry* geom = object->getGeometry();
		glm::mat4 transform = transformAccum.getMatrix();

		// uniform
		glUniformMatrix4fv(uniform_transform, 1, false, &transform[0][0]);
		glUniform4fv(uniform_color, 1, rgba);
		glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, label->getFontTexture());

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		geom->activate_position_uv();
		geom->activateIndexBuffer();
		geom->draw();
		geom->deactivate();
		geom->deactivateIndexBuffer();
		glDisable(GL_BLEND);
	}

	void OverlayPass_Text::setUniform_color(float newRGBA[4]) {
		memcpy(rgba, newRGBA, sizeof(float) * 4);
	}

}