#include "envmap.h"
#include "pathos/render/shader.h"
#include "glm/gtx/transform.hpp"
#include <string>

namespace pathos {

	/**
	* @param	textureID	cubemap texture id in which cubemap images are loaded. see pathos/loader/imageloader.h -> loadCubemapTexture().
	*/
	Skybox::Skybox(GLuint textureID) {
		this->textureID = textureID;
		createShader();
		cube = new CubeGeometry(glm::vec3(1.0f));
	}

	Skybox::~Skybox() {
		glDeleteProgram(program);
		delete cube;
	}

	void Skybox::createShader() {
		string vshader = R"(#version 430 core
layout (location = 0) in vec3 position;
uniform mat4 viewProj;
out VS_OUT { vec3 tc; } vs_out;
void main() {
  vs_out.tc = position;
  vs_out.tc.y *= -1;
  gl_Position = (viewProj * vec4(position, 1)).xyww;
}
)";

		string fshader = R"(#version 430 core
layout (binding = 0) uniform samplerCube texCube;
in VS_OUT { vec3 tc; } fs_in;
layout (location = 0) out vec4 color;
void main() {
  color = texture(texCube, fs_in.tc);
}
)";
		program = createProgram(vshader, fshader);
		if (program != 0) {
			uniform_transform = glGetUniformLocation(program, "viewProj");
		}
	}

	void Skybox::activate(const glm::mat4& transform) {
		glUseProgram(program);
		
		glUniformMatrix4fv(uniform_transform, 1, GL_FALSE, &transform[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_FRONT);

		cube->activatePositionBuffer(0);
		cube->activateIndexBuffer();
	}

	void Skybox::render() {
		cube->draw();
		
		// deactivate()
		cube->deactivateIndexBuffer();
		cube->deactivatePositionBuffer(0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glCullFace(GL_BACK);
		glUseProgram(0);
	}

}