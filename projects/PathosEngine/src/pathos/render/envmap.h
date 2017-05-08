#pragma once

#include <GL/glew.h>
#include "glm/glm.hpp"

namespace pathos {

	// Skybox render pass
	class Skybox {

	private:
		GLuint textureID;
		GLuint program;
		GLint uniform_viewTransform = -1;
		
		void createShader();

	public:
		Skybox(GLuint textureID);
		void activate(const glm::mat4& viewTransform);
		void render();

	};

}