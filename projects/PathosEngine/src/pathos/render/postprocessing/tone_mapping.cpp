#include "tone_mapping.h"
#include "pathos/console.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"

namespace pathos {

	class ToneMappingVS : public ShaderStage {
	public:
		ToneMappingVS() : ShaderStage(GL_VERTEX_SHADER, "ToneMappingVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class ToneMappingFS : public ShaderStage {
	public:
		ToneMappingFS() : ShaderStage(GL_FRAGMENT_SHADER, "ToneMappingFS") {
			setFilepath("tone_mapping.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_ToneMapping, ToneMappingVS, ToneMappingFS);

}

namespace pathos {

	static ConsoleVariable<float> cvar_tonemapping_exposure("r.tonemapping.exposure", 1.2f, "exposure parameter of tone mapping pass");
	static ConsoleVariable<float> cvar_gamma("r.gamma", 2.2f, "gamma correction");

	void ToneMapping::initializeResources(RenderCommandList& cmdList)
	{
		ubo.init<UBO_ToneMapping>();

		// tone mapping resource
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		//checkFramebufferStatus(cmdList, fbo); // #todo-framebuffer: Can't check completeness now
	}

	void ToneMapping::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void ToneMapping::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(ToneMapping);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneColor
		const GLuint input1 = getInput(EPostProcessInput::PPI_1); // sceneBloom
		const GLuint input2 = getInput(EPostProcessInput::PPI_2); // godRayResult
		const GLuint input3 = getInput(EPostProcessInput::PPI_3); // volumetricCloud
		const GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // toneMappingResult or backbuffer

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		if (output0 == 0) {
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		} else {
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);
		}

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_ToneMapping);
		cmdList.useProgram(program.getGLName());

		UBO_ToneMapping uboData;
		uboData.exposure = cvar_tonemapping_exposure.getValue();
		uboData.gamma    = cvar_gamma.getValue();
		ubo.update(cmdList, 0, &uboData);

		GLuint tonemapping_attachments[] = { input0, input1, input2, input3 };
		cmdList.bindTextures(0, 4, tonemapping_attachments);

		fullscreenQuad->drawPrimitive(cmdList);
	}

}
