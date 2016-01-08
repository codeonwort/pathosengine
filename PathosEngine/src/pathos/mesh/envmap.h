#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

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