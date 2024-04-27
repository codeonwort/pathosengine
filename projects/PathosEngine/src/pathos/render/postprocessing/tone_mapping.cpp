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
	static ConsoleVariable<float> cvar_tonemapping_exposureOverride("r.tonemapping.exposureOverride", 0.0f, "Control exposure if r.autoExposure is disabled");
	static ConsoleVariable<float> cvar_tonemapping_gamma("r.tonemapping.gamma", 2.2f, "gamma correction");

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

	struct UBO_ToneMapping {
		static constexpr uint32 BINDING_INDEX = 1;

		float exposureOverride;
		float gamma;
		int32 useAutoExposure;
		int32 sceneLuminanceLastMip;
		int32 applyBloom;
	};

}

namespace pathos {

	void ToneMapping::initializeResources(RenderCommandList& cmdList) {
		ubo.init<UBO_ToneMapping>();

		// tone mapping resource
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
	}

	void ToneMapping::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void ToneMapping::renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(ToneMapping);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneColor
		const GLuint input1 = getInput(EPostProcessInput::PPI_1); // sceneBloom
		const GLuint input2 = getInput(EPostProcessInput::PPI_2); // sceneLuminance
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
		uboData.exposureOverride      = cvar_tonemapping_exposureOverride.getValue();
		uboData.gamma                 = cvar_tonemapping_gamma.getValue();
		uboData.useAutoExposure       = (int32)bUseAutoExposure;
		uboData.sceneLuminanceLastMip = bUseAutoExposure ? (sceneContext.sceneLuminanceMipCount - 1) : 0;
		uboData.applyBloom            = (int32)bApplyBloom;
		ubo.update(cmdList, UBO_ToneMapping::BINDING_INDEX, &uboData);

		GLuint* colorAttachments = (GLuint*)cmdList.allocateSingleFrameMemory(sizeof(GLuint) * 3);
		colorAttachments[0] = input0;
		colorAttachments[1] = input1;
		colorAttachments[2] = input2;

		cmdList.bindTextures(0, 3, colorAttachments);

		fullscreenQuad->drawPrimitive(cmdList);
	}

}
