#include "depth_of_field.h"
#include <string>
#include <assert.h>

namespace pathos {

	DepthOfField::DepthOfField(unsigned int width, unsigned int height)
		:width(width), height(height)
	{
		createFBO();
		createShaders();
	}

	DepthOfField::~DepthOfField() {
		glDeleteTextures(2, texture_subsum2D);
		glDeleteProgram(program_subsum2D);
		glDeleteProgram(program_blur);
	}

	void DepthOfField::createFBO() {
		glGenTextures(2, texture_subsum2D);

		glBindTexture(GL_TEXTURE_2D, texture_subsum2D[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, height, width);
		glBindTexture(GL_TEXTURE_2D, texture_subsum2D[1]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	}

	void DepthOfField::createShaders() {
		// compute program. output is transposed.
		program_subsum2D = createSubsumShader();

		// program with vertex and fragment shaders.
		program_blur = createBlurShader();
	}

	void DepthOfField::render(GLuint texture_input) {
		//GLuint num_groups = (unsigned int)(ceil((float)width / 1024));
		glUseProgram(program_subsum2D);
		glBindImageTexture(0, texture_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, texture_subsum2D[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glDispatchCompute(height, 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		//glBindImageTexture(1, NULL, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // TODO: needed?
		glBindImageTexture(0, texture_subsum2D[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, texture_subsum2D[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glDispatchCompute(width, 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//glBindImageTexture(1, NULL, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // TODO: needed?

		/* texture_subsum2D[1] now holds subsum table */
		
		// apply box blur whose strength is relative to difference between pixel depth and focal depth
		glUseProgram(program_blur);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_subsum2D[1]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // render to backbuffer (assume: DOF is the final stage)
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	GLuint DepthOfField::createSubsumShader() {
		Shader cs(GL_COMPUTE_SHADER);
		cs.loadSource("subsum.glsl");

		GLuint program = pathos::createProgram(cs);
		assert(program);
		return program;
	}

	GLuint DepthOfField::createBlurShader() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("fullscreen_quad.glsl");
		fs.loadSource("depth_of_field.glsl");

		GLuint program = pathos::createProgram(vs, fs);
		assert(program);
		return program;
	}

}