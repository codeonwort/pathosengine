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

		std::string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
void main() {
	gl_Position = vec4(position, 1.0);
}
)";
		Shader vs(GL_VERTEX_SHADER, "VS_Bloom");
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER, "FS_Bloom");
		fs.loadSource("deferred_unpack_hdr.glsl");

		program = pathos::createProgram(vs, fs, "BlurPass");
		uniform_blur_horizontal = glGetUniformLocation(program, "horizontal");
	}

	void BloomPass::releaseResources(RenderCommandList& cmdList)
	{
		cmdList.deleteProgram(program);
		cmdList.deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void BloomPass::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(BloomPass);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.useProgram(program);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		// #todo-bloom: Use PPI_0 and PPI_1
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloomTemp, 0);
		cmdList.uniform1i(uniform_blur_horizontal, GL_TRUE);
		cmdList.bindTextureUnit(0, sceneContext.sceneBloom);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloom, 0);
		cmdList.uniform1i(uniform_blur_horizontal, GL_FALSE);
		cmdList.bindTextureUnit(0, sceneContext.sceneBloomTemp);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

}
