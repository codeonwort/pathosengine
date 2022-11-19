#include "bloom_setup.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_bloom_applyThreshold("r.bloom.applyThreshold", 0, "Apply threshold to sceneColor for bloom contribution");
	static ConsoleVariable<float> cvar_bloom_threshold("r.bloom.threshold", 1.0f, "Only sceneColor samples above the threshold contribute to bloom");
	static ConsoleVariable<float> cvar_bloom_exposure("r.bloom.exposure", 1.0f, "Exposure scaling");

	struct UBO_BloomSetup {
		static constexpr uint32 BINDING_POINT = 1;

		vector2 sceneSize;
		uint32 applyThreshold;
		float threshold;
		float exposure;
	};

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

		ubo.init<UBO_BloomSetup>("UBO_BloomSetup");
	}

	void BloomSetup::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void BloomSetup::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(BloomSetup);

		GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneColor
		GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // sceneBloomSetup

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_BloomSetup);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.viewport(0, 0, sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.useProgram(program.getGLName());
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);

		UBO_BloomSetup uboData;
		uboData.sceneSize.x = (float)sceneContext.sceneWidth;
		uboData.sceneSize.y = (float)sceneContext.sceneHeight;
		uboData.applyThreshold = (cvar_bloom_applyThreshold.getInt() != 0);
		uboData.threshold = cvar_bloom_threshold.getFloat();
		uboData.exposure = std::max(0.0f, cvar_bloom_exposure.getFloat());
		ubo.update(cmdList, UBO_BloomSetup::BINDING_POINT, &uboData);

		cmdList.bindTextureUnit(0, input0);
		cmdList.bindTextureUnit(1, sceneContext.gbufferA);
		cmdList.bindTextureUnit(2, sceneContext.gbufferB);
		cmdList.bindTextureUnit(3, sceneContext.gbufferC);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
	}

}
