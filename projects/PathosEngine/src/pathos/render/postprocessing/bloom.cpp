#include "bloom.h"
#include "pathos/shader/shader.h"
#include "pathos/render/scene_render_targets.h"

#include <string>

namespace pathos {

	void BloomPass::initializeResources(RenderCommandList& cmdList)
	{
		cmdList.createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		//checkFramebufferStatus(cmdList, fbo); // #todo-framebuffer: Can't check completeness now

		Shader vs(GL_VERTEX_SHADER, "VS_Bloom");
		vs.loadSource("fullscreen_quad.glsl");
		{
			Shader fs(GL_FRAGMENT_SHADER, "FS_Bloom");
			fs.addDefine("HORIZONTAL 1");
			fs.loadSource("two_pass_gaussian_blur.glsl");
			program = pathos::createProgram(vs, fs, "BlurPass_Horizontal");
		}
		{
			Shader fs(GL_FRAGMENT_SHADER, "FS_Bloom");
			fs.addDefine("HORIZONTAL 0");
			fs.loadSource("two_pass_gaussian_blur.glsl");
			program2 = pathos::createProgram(vs, fs, "BlurPass_Vertical");
		}
	}

	void BloomPass::releaseResources(RenderCommandList& cmdList)
	{
		cmdList.deleteProgram(program);
		cmdList.deleteProgram(program2);
		cmdList.deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void BloomPass::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(BloomPass);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneBloom
		const GLuint input1 = getInput(EPostProcessInput::PPI_1); // sceneBloomTemp

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		cmdList.useProgram(program);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, input1, 0);
		cmdList.bindTextureUnit(0, input0);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

		cmdList.useProgram(program2);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, input0, 0);
		cmdList.bindTextureUnit(0, input1);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
	}

}
