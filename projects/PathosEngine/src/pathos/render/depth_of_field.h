#pragma once

#include "pathos/shader/shader.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class DepthOfField {

	public:
		DepthOfField();
		~DepthOfField();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		// texture_input should contain rasterized image at level 0
		void render(RenderCommandList& cmdList, GLuint texture_input, GLuint targetFBO);

	private:
		GLuint createSubsumShader();
		GLuint createBlurShader();

		GLuint vao = 0;
		GLuint program_subsum2D = 0;
		GLuint program_blur = 0;
		UniformBuffer uboBlur;

		bool destroyed = false;

	};

}
