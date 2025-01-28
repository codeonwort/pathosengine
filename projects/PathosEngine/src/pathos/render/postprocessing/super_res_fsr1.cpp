#include "super_res_fsr1.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"

#include "badger/math/minmax.h"

// FSR1 consists of upscaling pass (EASU) and sharpening pass (RCAS).
// Order: Tone mapping -> EASU -> RCAS -> Noise-introducing PPs

// FSR requirements
// - Image should be well anti-aliased.
// - Input image must be normalized to [0,1] and be in perceptual space.
// - Image should be generated using negative MIP bias to increase texture detail.
// - Image should be noise-free.

// FSR quality modes
// - Ultra Quality : 1.3x per dimension
// - Quality       : 1.5x per dimension
// - Balanced      : 1.7x per dimension
// - Performance   : 2.0x per dimension

namespace pathos {

	static ConsoleVariable<float> cvar_fsr1_sharpness("r.fsr1.sharpness", 0.2f, "0.0 ~ 2.0 (The smaller, the sharper. Default is 0.2)");

	// 0 : Must use FP32 fallback.
	// 1 : FP16 mode is available with EXT extensions.
	// 2 : FP16 mode is available but NV extension is required.
	static int32 checkFP16Support() {
		const auto& ext = gRenderDevice->getExtensionSupport();
		if (ext.EXT_shader_16bit_storage && ext.EXT_shader_explicit_arithmetic_types) {
			return 1;
		}
		if (ext.NV_gpu_shader5) {
			return 2;
		}
		return 0;
	}

	struct UBO_FSR1 {
		static constexpr uint32 BINDING_POINT = 1;

		vector2ui renderViewportSize;
		vector2ui containerTextureSize;
		vector2ui upscaledViewportSize;
		float sharpness; // [0,2], 0 is sharpest
		float _padding0;
	};

	class FSR1_EASU : public ShaderStage {
	public:
		FSR1_EASU() : ShaderStage(GL_COMPUTE_SHADER, "FSR1_EASU") {
			const int32 fp16Criteria = checkFP16Support();
			addDefine("FP16_CRITERIA", fp16Criteria);
			setFilepath("fsr1/fsr1_wrapper_easu.glsl");
		}
	};

	class FSR1_RCAS : public ShaderStage {
	public:
		FSR1_RCAS() : ShaderStage(GL_COMPUTE_SHADER, "FSR1_RCAS") {
			const int32 fp16Criteria = checkFP16Support();
			addDefine("FP16_CRITERIA", fp16Criteria);
			setFilepath("fsr1/fsr1_wrapper_rcas.glsl");
		}
	};

	DEFINE_COMPUTE_PROGRAM(Program_FSR1_EASU, FSR1_EASU);
	DEFINE_COMPUTE_PROGRAM(Program_FSR1_RCAS, FSR1_RCAS);

}

namespace pathos {

	void FSR1::initializeResources(RenderCommandList& cmdList) {
		ubo.init<UBO_FSR1>("UBO_FSR1");
	}

	void FSR1::releaseResources(RenderCommandList& cmdList) {
		markDestroyed();
	}

	void FSR1::renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(FSR1);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // Previous sceneColor
		const GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // sceneColorUpscaled or sceneFinal
		CHECKF(output0 != 0, "Post processes should not write to the backbuffer");

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& programEASU = FIND_SHADER_PROGRAM(Program_FSR1_EASU);
		ShaderProgram& programRCAS = FIND_SHADER_PROGRAM(Program_FSR1_RCAS);

		vector2ui renderViewportSize(sceneContext.sceneWidth, sceneContext.sceneHeight);
		// NOTE: I'm always using the entire region of scene textures.
		//       Things might change if I implement Dynamic Resolution.
		vector2ui containerTextureSize = renderViewportSize;
		vector2ui upscaledViewportSize(sceneContext.sceneWidthSuperRes, sceneContext.sceneHeightSuperRes);

		UBO_FSR1 uboData;
		uboData.renderViewportSize = renderViewportSize;
		uboData.containerTextureSize = containerTextureSize;
		uboData.upscaledViewportSize = upscaledViewportSize;
		uboData.sharpness = badger::clamp(0.0f, cvar_fsr1_sharpness.getFloat(), 2.0f);
		ubo.update(cmdList, UBO_FSR1::BINDING_POINT, &uboData);

		constexpr GLenum PF_fsr1 = GL_RGBA16F;

		// EASU
		{
			cmdList.useProgram(programEASU.getGLName());

			cmdList.textureParameteri(input0, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			cmdList.bindTextureUnit(0, input0);
			cmdList.bindImageTexture(1, sceneContext.sceneColorUpscaledTemp, 0, GL_FALSE, 0, GL_READ_ONLY, PF_fsr1);

			// 'local_size_x' is 64, but we write 4 pixels per invocation.
			constexpr uint32 workRegionDim = 16;
			GLuint dispatchX = (upscaledViewportSize.x + (workRegionDim - 1)) / workRegionDim;
			GLuint dispatchY = (upscaledViewportSize.y + (workRegionDim - 1)) / workRegionDim;

			cmdList.dispatchCompute(dispatchX, dispatchY, 1);

			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
		// RCAS
		{
			cmdList.useProgram(programRCAS.getGLName());

			cmdList.bindImageTexture(0, sceneContext.sceneColorUpscaledTemp, 0, GL_FALSE, 0, GL_READ_ONLY, PF_fsr1);
			cmdList.bindImageTexture(1, output0, 0, GL_FALSE, 0, GL_READ_ONLY, PF_fsr1);

			// 'local_size_x' is 64, but we write 4 pixels per invocation.
			constexpr uint32 workRegionDim = 16;
			GLuint dispatchX = (upscaledViewportSize.x + (workRegionDim - 1)) / workRegionDim;
			GLuint dispatchY = (upscaledViewportSize.y + (workRegionDim - 1)) / workRegionDim;

			cmdList.dispatchCompute(dispatchX, dispatchY, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
	}

}
