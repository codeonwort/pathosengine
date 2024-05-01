#include "auto_exposure.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/mesh/geometry.h"
#include "pathos/util/engine_util.h"
#include "pathos/engine.h"
#include "pathos/console.h"

namespace pathos {

	static ConsoleVariable<float> cvar_exposure_minLogLuminance("r.exposure.minLogLuminance", -10.0f, "log2(minLuminance)");
	static ConsoleVariable<float> cvar_exposure_maxLogLuminance("r.exposure.maxLogLuminance", 20.0f, "log2(maxLuminance)");
	static ConsoleVariable<float> cvar_exposure_adaptationSpeed("r.exposure.adaptationSpeed", 1.1f, "Eye adaptation speed");

	constexpr uint32 HISTOGRAM_BIN_COUNT = 256;

	class AutoExposure_AverageLuminanceFS : public ShaderStage {
	public:
		AutoExposure_AverageLuminanceFS() : ShaderStage(GL_FRAGMENT_SHADER, "AutoExposure_AverageLuminanceFS") {
			setFilepath("auto_exposure_scene_avg.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_AutoExposure_AverageLuminance, FullscreenVS, AutoExposure_AverageLuminanceFS);

	struct UBO_HistogramGen {
		static constexpr uint32 BINDING_INDEX = 1;

		uint32 inputWidth;
		uint32 inputHeight;
		float  minLogLuminance;
		float  oneOverLogLuminanceRange;
	};

	class AutoExposure_HistogramGenCS : public ShaderStage {
	public:
		AutoExposure_HistogramGenCS() : ShaderStage(GL_COMPUTE_SHADER, "AutoExposure_HistogramGenCS") {
			setFilepath("auto_exposure_histogram_gen.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_AutoExposure_HistogramGen, AutoExposure_HistogramGenCS);

	struct UBO_HistogramAvg {
		static constexpr uint32 BINDING_INDEX = 1;

		uint32 pixelCount;
		float  minLogLuminance;
		float  logLuminanceRange;
		float  timeDelta;
		float  tau;
	};

	class AutoExposure_HistogramAvgCS : public ShaderStage {
	public:
		AutoExposure_HistogramAvgCS() : ShaderStage(GL_COMPUTE_SHADER, "AutoExposure_HistogramAvgCS") {
			setFilepath("auto_exposure_histogram_avg.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_AutoExposure_HistogramAvg, AutoExposure_HistogramAvgCS);

}

namespace pathos {

	AutoExposurePass::AutoExposurePass() {
	}

	AutoExposurePass::~AutoExposurePass() {
		CHECK(bDestroyed);
	}

	void AutoExposurePass::initializeResources(RenderCommandList& cmdList) {
		fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		luminanceHistogram = new Buffer(BufferCreateParams{ EBufferUsage::None, sizeof(uint32) * HISTOGRAM_BIN_COUNT, nullptr, "Buffer_LuminanceHisogram" });
		luminanceHistogram->createGPUResource_renderThread(cmdList);
		uboHistogramGen.init<UBO_HistogramGen>("UBO_LuminanceHistogramGen");
		uboHistogramAvg.init<UBO_HistogramAvg>("UBO_LuminanceHistogramAvg");
	}

	void AutoExposurePass::releaseResources(RenderCommandList& cmdList) {
		if (!bDestroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
			delete luminanceHistogram;
		}
		bDestroyed = true;
	}

	void AutoExposurePass::renderAutoExposure(RenderCommandList& cmdList, SceneProxy* scene, EAutoExposureMode mode) {
		SCOPED_DRAW_EVENT(AutoExposure);

		if (mode == EAutoExposureMode::Manual) {
			CHECKF(false, "[AutoExposure] Don't execute if using manual exposure");
		} else if (mode == EAutoExposureMode::SimpleAverage) {
			renderAutoExposure_averageLogLuminance(cmdList, scene);
		} else if (mode == EAutoExposureMode::Histogram) {
			renderAutoExposure_luminanceHistogram(cmdList, scene);
		}
	}

	void AutoExposurePass::getAutoExposureResults(
		const SceneRenderTargets& sceneContext, EAutoExposureMode mode,
		GLuint& outLuminanceTexture, uint32& outLuminanceTargetMip, bool& outLuminanceLogScale)
	{
		if (mode == EAutoExposureMode::Manual) {
			outLuminanceTexture = gEngine->getSystemTexture2DBlack()->internal_getGLName();
			outLuminanceTargetMip = 0;
			outLuminanceLogScale = false;
		} else if (mode == EAutoExposureMode::SimpleAverage) {
			outLuminanceTexture = sceneContext.sceneLuminance;
			outLuminanceTargetMip = sceneContext.sceneLuminanceMipCount - 1;
			outLuminanceLogScale = true;
		} else if (mode == EAutoExposureMode::Histogram) {
			outLuminanceTexture = sceneContext.luminanceFromHistogram;
			outLuminanceTargetMip = 0;
			outLuminanceLogScale = false;
		}
	}

	void AutoExposurePass::renderAutoExposure_averageLogLuminance(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(AverageLuminance);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const float minLogLuminance = cvar_exposure_minLogLuminance.getFloat();

		// Set shader program.
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AutoExposure_AverageLuminance);
		cmdList.useProgram(program.getGLName());

		// Set render target.
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneLuminance, 0);
		pathos::checkFramebufferStatus(cmdList, fbo, "[AutoExposure] FBO is invalid");

		// Set render states.
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.disable(GL_BLEND);
		cmdList.viewport(0, 0, sceneContext.sceneLuminanceSize, sceneContext.sceneLuminanceSize);

		// Set resource bindings.
		cmdList.uniform1f(1, minLogLuminance);
		cmdList.bindTextureUnit(0, sceneContext.sceneColor);

		// Drawcall.
		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		// Release resource bindings.
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		cmdList.bindTextureUnit(0, 0);

		// Generate mips. Smallest mip contains average scene luminance.
		cmdList.generateTextureMipmap(sceneContext.sceneLuminance);
	}

	void AutoExposurePass::renderAutoExposure_luminanceHistogram(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(LuminanceHistogram);

		const SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		const GLuint histogramBufferName = luminanceHistogram->internal_getGLName();

		const float minLogLuminance = cvar_exposure_minLogLuminance.getFloat();
		const float maxLogLuminance = cvar_exposure_maxLogLuminance.getFloat();
		const float logLuminanceRange = maxLogLuminance - minLogLuminance;
		const float timeDelta = scene->deltaSeconds;
		const float adaptationSpeed = cvar_exposure_adaptationSpeed.getFloat();

		// Clear histogram with zero values.
		{
			cmdList.clearNamedBufferData(histogramBufferName, GL_R32UI, GL_RED, GL_UNSIGNED_INT, (void*)0);
		}

		// Calculate luminance histogram.
		{
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AutoExposure_HistogramGen);
			cmdList.useProgram(program.getGLName());

			UBO_HistogramGen uboData;
			uboData.inputWidth               = sceneContext.sceneWidth;
			uboData.inputHeight              = sceneContext.sceneHeight;
			uboData.minLogLuminance          = minLogLuminance;
			uboData.oneOverLogLuminanceRange = 1.0f / logLuminanceRange;
			uboHistogramGen.update(cmdList, UBO_HistogramGen::BINDING_INDEX, &uboData);

			cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, histogramBufferName);
			cmdList.bindTextureUnit(0, sceneContext.sceneColor);

			const uint32 dispatchX = (sceneContext.sceneWidth + 15) / 16;
			const uint32 dispatchY = (sceneContext.sceneHeight + 15) / 16;
			cmdList.dispatchCompute(dispatchX, dispatchY, 1);
			cmdList.memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // For histogram SSBO
		}

		// Calculate average of histogram.
		{
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AutoExposure_HistogramAvg);
			cmdList.useProgram(program.getGLName());

			UBO_HistogramAvg uboData;
			uboData.pixelCount        = (uint32)(sceneContext.sceneWidth * sceneContext.sceneHeight);
			uboData.minLogLuminance   = minLogLuminance;
			uboData.logLuminanceRange = logLuminanceRange;
			uboData.timeDelta         = timeDelta;
			uboData.tau               = adaptationSpeed;
			uboHistogramAvg.update(cmdList, UBO_HistogramAvg::BINDING_INDEX, &uboData);

			cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, histogramBufferName);
			cmdList.bindImageTexture(0, sceneContext.luminanceFromHistogram, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
			
			cmdList.dispatchCompute(16, 16, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // For luminance texture
		}
	}

}
