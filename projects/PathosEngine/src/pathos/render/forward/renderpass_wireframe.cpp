#include "renderpass_wireframe.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	WireframePass::WireframePass() {
		createProgram();
	}

	void WireframePass::createProgram() {
		fsSource.uniform("vec4", "color");
		fsSource.outVar("vec4", "out_color");
		fsSource.mainCode("out_color = color;");

		dumpShaderSource(vsSource, "renderpass_wireframe.vert");
		dumpShaderSource(fsSource, "renderpass_wireframe.frag");

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode());

		uniform_mvp = glGetUniformLocation(program, "mvpTransform");
		uniform_color = glGetUniformLocation(program, "color");
	}

	void WireframePass::render(Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		static_cast<void>(scene);
		WireframeMaterial* material = static_cast<WireframeMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position();
		geometry->activateIndexBuffer();

		glUseProgram(program);

		//glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(uniform_mvp, 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		glUniform4fv(uniform_color, 1, material->getColor());

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		geometry->draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivate();
		geometry->deactivateIndexBuffer();
		glDisable(GL_BLEND);

		glUseProgram(0);
	}

}