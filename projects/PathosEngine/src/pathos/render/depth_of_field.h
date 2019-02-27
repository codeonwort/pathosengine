#pragma once

#include "pathos/shader/shader.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class DepthOfField {

	public:
		DepthOfField(uint32_t width, uint32_t height);
		~DepthOfField();

		// texture_input should contain rasterized image at level 0
		void render(GLuint texture_input, GLuint targetFBO);

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
		UniformBuffer uboBlur;

		uint32_t width;
		uint32_t height;

	};

}