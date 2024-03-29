#include "tone_mapping.h"

#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/util/engine_util.h"
#include "pathos/console.h"

#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_tonemapping_operator("r.tonemapping.operator", 1, "0 = Reinhard, 1 = ACES");
	static ConsoleVariable<float> cvar_tonemapping_exposure("r.tonemapping.exposure", 1.2f, "exposure parameter of tone mapping pass");
	static ConsoleVariable<float> cvar_gamma("r.gamma", 2.2f, "gamma correction");

	template<int32 ToneMapper>
	class ToneMappingFS : public ShaderStage {
	public:
		ToneMappingFS() : ShaderStage(GL_FRAGMENT_SHADER, "ToneMappingFS") {
			addDefine("TONE_MAPPER", ToneMapper);
			setFilepath("tone_mapping.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_ToneMapping_Reinhard, FullscreenVS, ToneMappingFS<0>);
	DEFINE_SHADER_PROGRAM2(Program_ToneMapping_ACES, FullscreenVS, ToneMappingFS<1>);

	ShaderProgram& getToneMapingProgram() {
		int32 op = badger::clamp(0, cvar_tonemapping_operator.getInt(), 1);
		if (op == 0) {
			return FIND_SHADER_PROGRAM(Program_ToneMapping_Reinhard);
		} else {
			return FIND_SHADER_PROGRAM(Program_ToneMapping_ACES);
		}
	}

}

namespace pathos {

	void ToneMapping::initializeResources(RenderCommandList& cmdList)
	{
		ubo.init<UBO_ToneMapping>();

		// tone mapping resource
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
	}

	void ToneMapping::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void ToneMapping::renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad)
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
			pathos::checkFramebufferStatus(cmdList, fbo, "toneMapping");
		}

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		ShaderProgram& program = getToneMapingProgram();
		cmdList.useProgram(program.getGLName());

		UBO_ToneMapping uboData;
		uboData.exposure = cvar_tonemapping_exposure.getValue();
		uboData.gamma    = cvar_gamma.getValue();
		ubo.update(cmdList, 1, &uboData);

		GLuint* colorAttachments = (GLuint*)cmdList.allocateSingleFrameMemory(sizeof(GLuint) * 4);
		colorAttachments[0] = input0;
		colorAttachments[1] = input1;
		colorAttachments[2] = input2;
		colorAttachments[3] = input3;

		cmdList.bindTextures(0, 4, colorAttachments);

		fullscreenQuad->drawPrimitive(cmdList);
	}

}
