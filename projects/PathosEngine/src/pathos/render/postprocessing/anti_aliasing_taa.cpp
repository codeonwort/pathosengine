#include "anti_aliasing_taa.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/util/engine_util.h"
#include "pathos/console.h"
#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<float> cvar_taaHistoryWeight("r.taa.historyWeight", 0.9f, "sceneColor history weight");

	struct UBO_TAA {
		static constexpr uint32 BINDING_POINT = 1;
		
		float historyWeight;
	};

	class TemporalAntiAliasingVS : public ShaderStage {
	public:
		TemporalAntiAliasingVS() : ShaderStage(GL_VERTEX_SHADER, "TemporalAntiAliasingVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class TemporalAntiAliasingFS : public ShaderStage {
	public:
		TemporalAntiAliasingFS() : ShaderStage(GL_FRAGMENT_SHADER, "TemporalAntiAliasingFS") {
			setFilepath("temporal_anti_aliasing.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_TAA, TemporalAntiAliasingVS, TemporalAntiAliasingFS);

}

namespace pathos {

	void TAA::initializeResources(RenderCommandList& cmdList) {
		ubo.init<UBO_TAA>("UBO_TAA");

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_TAA");
	}

	void TAA::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		markDestroyed();
	}

	void TAA::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(TAA);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneColorToneMapped
		const GLuint input1 = getInput(EPostProcessInput::PPI_1); // sceneColorHistory
		const GLuint input2 = getInput(EPostProcessInput::PPI_2); // sceneDepth
		const GLuint input3 = getInput(EPostProcessInput::PPI_3); // velocityMap
		const GLuint input4 = getInput(EPostProcessInput::PPI_4); // gbufferA
		const GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // sceneColorAA or sceneFinal
		CHECKF(output0 != 0, "Post processes do not write to the backbuffer anymore");

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_TAA);
		cmdList.useProgram(program.getGLName());

		UBO_TAA uboData;
		uboData.historyWeight = badger::clamp(0.0f, cvar_taaHistoryWeight.getFloat(), 1.0f);
		ubo.update(cmdList, UBO_TAA::BINDING_POINT, &uboData);

		cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		cmdList.textureParameteri(input0, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(input0, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureParameteri(input1, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(input1, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureParameteri(input3, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.textureParameteri(input3, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		cmdList.bindTextureUnit(0, input0);
		cmdList.bindTextureUnit(1, input1);
		cmdList.bindTextureUnit(2, input2);
		cmdList.bindTextureUnit(3, input3);
		cmdList.bindTextureUnit(4, input4);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
	}

}
