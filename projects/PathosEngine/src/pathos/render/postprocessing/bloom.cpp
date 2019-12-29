#include "bloom.h"
#include "pathos/shader/shader.h"
#include "pathos/render/scene_render_targets.h"

#include <string>

namespace pathos {

	void BloomPass::initializeResources(RenderCommandList& cmdList)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.createFramebuffers(1, &fbo);
		// #todo-framebuffer: This is set twice each with different texture. Any performance issue? Maybe I need two FBOs?
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloomTemp, 0);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		checkFramebufferStatus(cmdList, fbo);

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

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		// #todo-bloom: Use PPI_0 and PPI_1
		cmdList.useProgram(program);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloomTemp, 0);
		cmdList.bindTextureUnit(0, sceneContext.sceneBloom);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

		cmdList.useProgram(program2);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloom, 0);
		cmdList.bindTextureUnit(0, sceneContext.sceneBloomTemp);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
	}

}
