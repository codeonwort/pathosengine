#include "bloom_setup.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"

namespace pathos {

	class BloomSetupVS : public ShaderStage {
	public:
		BloomSetupVS() : ShaderStage(GL_VERTEX_SHADER, "BloomSetupVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class BloomSetupFS : public ShaderStage {
	public:
		BloomSetupFS() : ShaderStage(GL_FRAGMENT_SHADER, "BloomSetupFS")
		{
			setFilepath("bloom_setup.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_BloomSetup, BloomSetupVS, BloomSetupFS);

}

namespace pathos {

	void BloomSetup::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_BloomSetup");
	}

	void BloomSetup::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void BloomSetup::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(BloomSetup);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneColor
		GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // sceneBloom

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_BloomSetup);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.useProgram(program.getGLName());
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);
		cmdList.bindTextureUnit(0, input0);
		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
	}

}
