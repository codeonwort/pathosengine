#include "bloom.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"

#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_bloom_iterations("r.bloom.iteration", 5, "Bloom pass iteration count");

	class BloomVS : public ShaderStage {
	public:
		BloomVS() : ShaderStage(GL_VERTEX_SHADER, "BloomVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class BloomHorizontalFS : public ShaderStage {
	public:
		BloomHorizontalFS() : ShaderStage(GL_FRAGMENT_SHADER, "BloomHorizontalFS")
		{
			addDefine("HORIZONTAL 1");
			addDefine("KERNEL_SIZE 7");
			setFilepath("two_pass_gaussian_blur.glsl");
		}
	};

	class BloomVerticalFS : public ShaderStage {
	public:
		BloomVerticalFS() : ShaderStage(GL_FRAGMENT_SHADER, "BloomVerticalFS")
		{
			addDefine("HORIZONTAL 0");
			addDefine("KERNEL_SIZE 7");
			setFilepath("two_pass_gaussian_blur.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_BloomHorizontal, BloomVS, BloomHorizontalFS);
	DEFINE_SHADER_PROGRAM2(Program_BloomVertical, BloomVS, BloomVerticalFS);

}

namespace pathos {

	void BloomPass::initializeResources(RenderCommandList& cmdList)
	{
		cmdList.createFramebuffers(2, fbo);
		cmdList.namedFramebufferDrawBuffer(fbo[0], GL_COLOR_ATTACHMENT0);
		cmdList.namedFramebufferDrawBuffer(fbo[1], GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo[0], -1, "FBO_BloomPass[0]");
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo[1], -1, "FBO_BloomPass[1]");
		//checkFramebufferStatus(cmdList, fbo); // #todo-framebuffer: Can't check completeness now
	}

	void BloomPass::releaseResources(RenderCommandList& cmdList)
	{
		cmdList.deleteFramebuffers(2, fbo);

		markDestroyed();
	}

	void BloomPass::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(BloomPass);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneBloom
		const GLuint input1 = getInput(EPostProcessInput::PPI_1); // sceneBloomTemp

		ShaderProgram& program_horizontal = FIND_SHADER_PROGRAM(Program_BloomHorizontal);
		ShaderProgram& program_vertical = FIND_SHADER_PROGRAM(Program_BloomHorizontal);

		int32 count = clamp(0, cvar_bloom_iterations.getValue(), 8);

		while (count --> 0) {
			cmdList.useProgram(program_horizontal.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[0]);
			cmdList.namedFramebufferTexture(fbo[0], GL_COLOR_ATTACHMENT0, input1, 0);
			cmdList.bindTextureUnit(0, input0);
			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);
			cmdList.namedFramebufferTexture(fbo[0], GL_COLOR_ATTACHMENT0, 0, 0);

			cmdList.useProgram(program_vertical.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
			cmdList.namedFramebufferTexture(fbo[1], GL_COLOR_ATTACHMENT0, input0, 0);
			cmdList.bindTextureUnit(0, input1);
			fullscreenQuad->drawPrimitive(cmdList);
			cmdList.namedFramebufferTexture(fbo[1], GL_COLOR_ATTACHMENT0, 0, 0);
		}
	}

}
