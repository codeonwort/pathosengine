#pragma once

#include <GL/glew.h>
#include "glm/glm.hpp"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	// Skybox render pass
	class Skybox {

	public:
		Skybox(GLuint textureID);
		~Skybox();
		void activate(const glm::mat4& viewTransform);
		void render();

	private:
		GLuint textureID;
		GLuint program;
		GLint uniform_transform = -1;
		CubeGeometry* cube;
		
		void createShader();

	};

}