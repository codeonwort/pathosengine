#pragma once

#include "pathos/shader/shader.h"

namespace pathos {

	class DepthOfField {

	public:
		DepthOfField(unsigned int width, unsigned int height);
		~DepthOfField();

		// texture_input should contain rasterized image at level 0
		void render(GLuint texture_input);

	private:
		void createFBO();
		void createShaders();
		GLuint createSubsumShader();
		GLuint createBlurShader();

		GLuint vao;
		GLuint fbo;
		GLuint texture_subsum2D[2];
		GLuint program_subsum2D;
		GLuint program_blur;

		unsigned int width, height;

	};

}