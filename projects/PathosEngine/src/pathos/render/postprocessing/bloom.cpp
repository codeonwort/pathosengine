#include "bloom.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"
#include "pathos/engine.h"
#include "pathos/util/engine_util.h"

#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_bloom_preset("r.bloom.preset", 2, "Select bloom weights preset");

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
			addDefine("HORIZONTAL", 1);
			addDefine("KERNEL_SIZE", 3);
			setFilepath("two_pass_gaussian_blur.glsl");
		}
	};

	class BloomVerticalFS : public ShaderStage {
	public:
		BloomVerticalFS() : ShaderStage(GL_FRAGMENT_SHADER, "BloomVerticalFS")
		{
			addDefine("HORIZONTAL", 0);
			addDefine("KERNEL_SIZE", 3);
			addDefine("ADDITIVE", 1);
			setFilepath("two_pass_gaussian_blur.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_BloomHorizontal, BloomVS, BloomHorizontalFS);
	DEFINE_SHADER_PROGRAM2(Program_BloomVertical, BloomVS, BloomVerticalFS);

}

namespace pathos {

	void BloomPass::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_BloomPass");
	}

	void BloomPass::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void BloomPass::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(BloomPass);

		static constexpr uint32 MAX_ADDITIVE_WEIGHTS = 5;
		static const float additiveWeightsPreset0[MAX_ADDITIVE_WEIGHTS] = {
			1.0f,
			1.0f,
			1.0f,
			1.0f,
			1.0f
		};
		static const float additiveWeightsPreset1[MAX_ADDITIVE_WEIGHTS] = {
			1.0f,
			0.5f,
			0.25f,
			0.125f,
			0.125f
		};
		static const float additiveWeightsPreset2[MAX_ADDITIVE_WEIGHTS] = {
			0.5f,
			0.25f,
			0.125f,
			0.0625f,
			0.03125f
		};
		static const float* additiveWeightsPtr[3] = { additiveWeightsPreset0, additiveWeightsPreset1, additiveWeightsPreset2 };
		int32 bloomPresetIndex = badger::clamp(0, cvar_bloom_preset.getInt(), 2);
		const float* additiveWeights = additiveWeightsPtr[bloomPresetIndex];

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneBloom
		GLuint input1 = getInput(EPostProcessInput::PPI_1); // sceneBloomTemp

		uint32 downsampleCount = sceneContext.sceneColorDownsampleMipmapCount;
		std::vector<GLuint>& sceneColorDownsampleViews = sceneContext.sceneColorDownsampleViews;
		std::vector<GLuint>& sceneBloomTempViews = sceneContext.sceneBloomTempViews;

		CHECK(downsampleCount <= MAX_ADDITIVE_WEIGHTS + 1);

		ShaderProgram& program_horizontal = FIND_SHADER_PROGRAM(Program_BloomHorizontal);
		ShaderProgram& program_vertical = FIND_SHADER_PROGRAM(Program_BloomVertical);

		uint32 viewportWidth = sceneContext.sceneWidth / 2;
		uint32 viewportHeight = sceneContext.sceneHeight / 2;

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		for (uint32 i = 0; i < downsampleCount; ++i) {
			int32 mipIndex = downsampleCount - i - 1;
			GLuint sceneColorMip = sceneColorDownsampleViews[mipIndex];
			GLuint bloomTempMip = sceneBloomTempViews[mipIndex];
			GLuint bloomMip = sceneContext.sceneBloomViews[mipIndex];
			GLuint bloomMipPrev = (mipIndex == downsampleCount - 1) ? gEngine->getSystemTexture2DBlack() : sceneContext.sceneBloomViews[mipIndex + 1];

			cmdList.viewport(0, 0, viewportWidth >> mipIndex, viewportHeight >> mipIndex);

			cmdList.useProgram(program_horizontal.getGLName());
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, bloomTempMip, 0);
			pathos::checkFramebufferStatus(cmdList, fbo, "bloomTempMip");

			cmdList.bindTextureUnit(0, sceneColorMip);
			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);

			cmdList.useProgram(program_vertical.getGLName());
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, bloomMip, 0);
			pathos::checkFramebufferStatus(cmdList, fbo, "bloomMip");
			cmdList.bindTextureUnit(0, bloomTempMip);
			cmdList.bindTextureUnit(1, bloomMipPrev);
			cmdList.uniform1f(0, additiveWeights[mipIndex]);
			fullscreenQuad->drawPrimitive(cmdList);
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
		}
	}

}
