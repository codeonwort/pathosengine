#include "overlaypass_standard.h"
#include "pathos/render/shader.h"

namespace pathos {

	OverlayPass_Standard::OverlayPass_Standard() {
		createProgram();
	}

	void OverlayPass_Standard::createProgram() {
		std::string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

uniform mat4 transform;

out VS_OUT {
	vec2 uv;
} vs_out;

void main() {
	vs_out.uv = uv;
	gl_Position = transform * vec4(position, 1.0f);
}

)";

		std::string fshader = R"(
#version 430 core

layout (location = 0) in vec3 uv;

uniform vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
uniform sampler2D tex_sampler;

in VS_OUT {
	vec2 uv;
} fs_in;

out vec4 out_color;

void main() {
	//out_color = texture(tex_sampler, fs_in.uv) + color;
	out_color = color;
}

)";
		program = pathos::createProgram(vshader, fshader);
		uniform_transform = glGetUniformLocation(program, "transform");
	}

	void OverlayPass_Standard::render(DisplayObject2D* object, const Transform& transformAccum) {
		glUseProgram(program);

		MeshGeometry* geom = object->getGeometry();
		glm::mat4 transform = transformAccum.getMatrix() * object->getTransform().getMatrix();

		// uniform
		glUniformMatrix4fv(uniform_transform, 1, false, &transform[0][0]);
		//glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT);
		//glBindTexture(GL_TEXTURE_2D, 0);

		// geometry
		geom->activatePositionBuffer(0);
		geom->activateUVBuffer(1);
		geom->activateIndexBuffer();

		// draw call
		geom->draw();

		// release
		geom->deactivatePositionBuffer(0);
		geom->deactivateUVBuffer(1);
		geom->deactivateIndexBuffer();


	}

}