#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace pathos {

	class Skybox {
	private:
		GLuint textureID, program;
	public:
		Skybox(GLuint textureID);
		void activate(const glm::mat4& viewTransform);
		void render();
	};

}