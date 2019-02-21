#pragma once

#include "gl_core.h"
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
		GLuint program;
		GLuint textureID;
		GLint uniform_transform = -1;

		CubeGeometry* cube;
		
		void createShader();

	};

}
