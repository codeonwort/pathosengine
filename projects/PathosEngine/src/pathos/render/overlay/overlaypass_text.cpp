#include "overlaypass_text.h"
#include "pathos/overlay/label.h"
#include "pathos/shader/shader.h"

namespace pathos {

	static constexpr unsigned int TEXTURE_UNIT = 0;

	OverlayPass_Text::OverlayPass_Text() {
		createProgram();
	}

	OverlayPass_Text::~OverlayPass_Text() {
		glDeleteProgram(program);
	}

	void OverlayPass_Text::createProgram() {
		Shader vs(GL_VERTEX_SHADER, "VS_OverlayPass_Text");
		Shader fs(GL_FRAGMENT_SHADER, "FS_OverlayPass_Text");
		vs.loadSource("overlay_text_vs.glsl");
		fs.loadSource("overlay_text_fs.glsl");
		program = pathos::createProgram(vs, fs, "OverlayPass_Text");

#define GETUNIFORM(uname) { uniform_##uname = glGetUniformLocation(program, #uname); assert(uniform_##uname != -1); }
		GETUNIFORM(transform);
		GETUNIFORM(color);
#undef GETUNIFORM
	}

	void OverlayPass_Text::renderOverlay(RenderCommandList& cmdList, DisplayObject2D* object, const Transform& transformAccum) {
		Label* label = static_cast<Label*>(object);

		cmdList.useProgram(program);

		MeshGeometry* geom = object->getGeometry();
		glm::mat4 transform = transformAccum.getMatrix();

		// uniform
		cmdList.uniformMatrix4fv(uniform_transform, 1, false, &transform[0][0]);
		cmdList.uniform4fv(uniform_color, 1, rgba);
		cmdList.bindTextureUnit(TEXTURE_UNIT, label->getFontTexture());

		cmdList.enable(GL_BLEND);
		cmdList.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		geom->activate_position_uv(cmdList);
		geom->activateIndexBuffer(cmdList);
		geom->drawPrimitive(cmdList);
		geom->deactivate(cmdList);
		geom->deactivateIndexBuffer(cmdList);
		cmdList.disable(GL_BLEND);
	}

	void OverlayPass_Text::setUniform_color(float newRGBA[4]) {
		memcpy_s(rgba, sizeof(float) * 4, newRGBA, sizeof(float) * 4);
	}

}
