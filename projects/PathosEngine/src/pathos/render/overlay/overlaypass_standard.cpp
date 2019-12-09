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
		Shader vs(GL_VERTEX_SHADER, "VS_OverlayPass_Standard");
		Shader fs(GL_FRAGMENT_SHADER, "FS_OverlayPass_Standard");
		vs.loadSource("overlay_standard_vs.glsl");
		fs.loadSource("overlay_standard_fs.glsl");
		program = pathos::createProgram(vs, fs, "OverlayPass_Standard");

#define GETUNIFORM(uname) { uniform_##uname = glGetUniformLocation(program, #uname); assert(uniform_##uname != -1); }
		GETUNIFORM(transform);
		GETUNIFORM(color);
#undef GETUNIFORM
	}

	void OverlayPass_Standard::renderOverlay(RenderCommandList& cmdList, DisplayObject2D* object, const Transform& transformAccum) {
		MeshGeometry* geom = object->getGeometry();
		glm::mat4 transform = transformAccum.getMatrix();

		cmdList.useProgram(program);

		// uniform
		cmdList.uniformMatrix4fv(uniform_transform, 1, false, &transform[0][0]);
		cmdList.uniform4fv(uniform_color, 1, rgba);
		//glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT);
		//glBindTexture(GL_TEXTURE_2D, 0);

		geom->activate_position_uv(cmdList);
		geom->activateIndexBuffer(cmdList);
		geom->drawPrimitive(cmdList);
		geom->deactivate(cmdList);
		geom->deactivateIndexBuffer(cmdList);
	}

	void OverlayPass_Standard::setUniform_color(float newRGBA[4]) {
		memcpy_s(rgba, sizeof(float) * 4, newRGBA, sizeof(float) * 4);
	}

}
