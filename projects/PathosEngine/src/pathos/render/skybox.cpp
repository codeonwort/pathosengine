#include "skybox.h"
#include "scene.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include <string>

namespace pathos {

	/**
	* @param	textureID	see pathos/loader/imageloader.h -> loadCubemapTexture().
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
		std::string vshader = R"(#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 viewProj;

out VS_OUT { vec3 tc; } vs_out;

void main() {
  vs_out.tc = position;
  vs_out.tc.y *= -1;
  gl_Position = (viewProj * vec4(position, 1)).xyww;
}
)";

		std::string fshader = R"(#version 430 core
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

	void Skybox::render(const Scene* scene, const Camera* camera) {
		SCOPED_DRAW_EVENT(Skybox);

		glm::mat4 view = glm::mat4(glm::mat3(camera->getViewMatrix())); // view transform without transition
		glm::mat4 proj = camera->getProjectionMatrix();
		glm::mat4 transform = proj * view;

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_DEPTH_TEST);
		glCullFace(GL_FRONT);

		glUseProgram(program);
		glUniformMatrix4fv(uniform_transform, 1, GL_FALSE, &transform[0][0]);
		glBindTextureUnit(0, textureID);

		cube->activate_position();
		cube->activateIndexBuffer();

		cube->draw();
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glCullFace(GL_BACK);
	}

}
