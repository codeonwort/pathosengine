#include "ssao.h"
#include "pathos/console.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/util/engine_util.h"

#include "badger/math/random.h"
#include "badger/math/minmax.h"

// Based on "Robust Screen Space Ambient Occlusion in 1 ms in 1080p on PS4"
// (Wojciech Sterna, GPU Zen)

// #todo-ssao: Upsample the blurred SSAO with a bilateral filter.
// But... looks not bad with mere bilinear filtering
// and I don't wanna make an additional full-resolution texture.

namespace pathos {

	static constexpr GLuint UBO_SSAO_BINDING_POINT = 1;
	static constexpr GLuint UBO_SSAO_RANDOM_BINDING_POINT = 2;

	static ConsoleVariable<int32> cvar_ssao_enable("r.ssao.enable", 1, "Enable SSAO");
	static ConsoleVariable<int32> cvar_ssao_spp("r.ssao.samplesPerPixel", 32, "Determines SPP(Samples Per Pixel)");
	static ConsoleVariable<float> cvar_ssao_worldRadius("r.ssao.worldRadius", 5.0f, "World radius of sample space");
	static ConsoleVariable<float> cvar_ssao_maxScreenRadius("r.ssao.maxScreenRadius", 0.2f, "Screen space max radius");
	static ConsoleVariable<float> cvar_ssao_contrast("r.ssao.contrast", 1.0f, "Contrast of AO effect");

	// For SSAO_Compute
	struct UBO_SSAO {
		uint32 spp;
		float worldRadius;
		float maxScreenRadius;
		float contrast;
	};

	class SSAO_Downscale : public ShaderStage {
	public:
		SSAO_Downscale() : ShaderStage(GL_COMPUTE_SHADER, "SSAO_Downscale") {
			setFilepath("ssao_downscale.glsl");
		}
	};

	class SSAO_Compute : public ShaderStage {
	public:
		SSAO_Compute() : ShaderStage(GL_COMPUTE_SHADER, "SSAO_Compute") {
			addDefine("SSAO_NUM_ROTATION_NOISE", SSAO_NUM_ROTATION_NOISE);
			setFilepath("ssao_ao.glsl");
		}
	};

	class SSAO_BlurVS : public ShaderStage {
	public:
		SSAO_BlurVS() : ShaderStage(GL_VERTEX_SHADER, "SSAO_BlurVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class SSAO_BlurHorizontalFS : public ShaderStage {
	public:
		SSAO_BlurHorizontalFS() : ShaderStage(GL_FRAGMENT_SHADER, "SSAO_BlurHorizontalFS") {
			addDefine("HORIZONTAL", 1);
			setFilepath("ssao_blur.glsl");
		}
	};

	class SSAO_BlurVerticalFS : public ShaderStage {
	public:
		SSAO_BlurVerticalFS() : ShaderStage(GL_FRAGMENT_SHADER, "SSAO_BlurVerticalFS") {
			addDefine("HORIZONTAL", 0);
			setFilepath("ssao_blur.glsl");
		}
	};

	DEFINE_COMPUTE_PROGRAM(Program_SSAO_Downscale, SSAO_Downscale);
	DEFINE_COMPUTE_PROGRAM(Program_SSAO_Compute, SSAO_Compute);
	DEFINE_SHADER_PROGRAM2(Program_SSAO_BlurHorizontal, SSAO_BlurVS, SSAO_BlurHorizontalFS);
	DEFINE_SHADER_PROGRAM2(Program_SSAO_BlurVertical, SSAO_BlurVS, SSAO_BlurVerticalFS);

}

namespace pathos {

	void SSAO::initializeResources(RenderCommandList& cmdList)
	{
		ubo.init<UBO_SSAO>();
		uboRandom.init<UBO_SSAO_Random>();

		gRenderDevice->createFramebuffers(1, &fboBlur);
		cmdList.namedFramebufferDrawBuffer(fboBlur, GL_COLOR_ATTACHMENT0);

		gRenderDevice->createFramebuffers(1, &fboBlur2);
		cmdList.namedFramebufferDrawBuffer(fboBlur2, GL_COLOR_ATTACHMENT0);
	}

	void SSAO::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fboBlur);
		gRenderDevice->deleteFramebuffers(1, &fboBlur2);

		markDestroyed();
	}

	void SSAO::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(SSAO);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		if (cvar_ssao_enable.getInt() == 0) {
			GLfloat* clearValue = (GLfloat*)cmdList.allocateSingleFrameMemory(sizeof(GLfloat) * 4);
			for (int32 i = 0; i < 4; ++i) clearValue[i] = 1.0f;
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur2);
			cmdList.namedFramebufferTexture(fboBlur2, GL_COLOR_ATTACHMENT0, sceneContext.ssaoMap, 0);
			cmdList.clearBufferfv(GL_COLOR, 0, clearValue);
			return;
		}

		{
			SCOPED_DRAW_EVENT(SSAODownsample);

			GLuint workGroupsX = (GLuint)ceilf((float)(sceneContext.sceneWidth / 2) / 64.0f);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_SSAO_Downscale);
			cmdList.useProgram(program.getGLName());
			
			cmdList.bindTextureUnit(0, sceneContext.sceneDepth);
			cmdList.bindImageTexture(1, sceneContext.gbufferA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32UI);
			cmdList.bindImageTexture(2, sceneContext.gbufferB, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			cmdList.bindImageTexture(3, sceneContext.ssaoHalfNormalAndDepth, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			cmdList.dispatchCompute(workGroupsX, sceneContext.sceneHeight / 2, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		{
			SCOPED_DRAW_EVENT(SSAOCompute);

			GLuint workGroupsX = (GLuint)ceilf((float)(sceneContext.sceneWidth / 2) / 16.0f);
			GLuint workGroupsY = (GLuint)ceilf((float)(sceneContext.sceneHeight / 2) / 16.0f);

			ShaderProgram& program_computeAO = FIND_SHADER_PROGRAM(Program_SSAO_Compute);
			cmdList.useProgram(program_computeAO.getGLName());

			UBO_SSAO uboData;
			uboData.spp             = badger::clamp(1u, (uint32)cvar_ssao_spp.getInt(), SSAO_MAX_SAMPLE_POINTS);
			uboData.worldRadius     = cvar_ssao_worldRadius.getFloat();
			uboData.maxScreenRadius = cvar_ssao_maxScreenRadius.getFloat();
			uboData.contrast        = cvar_ssao_contrast.getFloat();
			ubo.update(cmdList, UBO_SSAO_BINDING_POINT, &uboData);

			if (bRandomDataValid == false) {
				for (uint32 i = 0; i < SSAO_NUM_ROTATION_NOISE; ++i) {
					vector4 v(Random() * 2.0f - 1.0f, Random() * 2.0f - 1.0f, 0.0f, 0.0f);
					randomData.randomRotations[i] = v;
				}
				bRandomDataValid = true;
			}
			uboRandom.update(cmdList, UBO_SSAO_RANDOM_BINDING_POINT, &randomData);

			cmdList.bindImageTexture(0, sceneContext.ssaoHalfNormalAndDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
			cmdList.bindImageTexture(1, sceneContext.ssaoMap, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
			cmdList.dispatchCompute(workGroupsX, workGroupsY, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		{
			SCOPED_DRAW_EVENT(SSAOBlur);

			ShaderProgram& program_horizontal = FIND_SHADER_PROGRAM(Program_SSAO_BlurHorizontal);
			ShaderProgram& program_vertical = FIND_SHADER_PROGRAM(Program_SSAO_BlurVertical);

			cmdList.viewport(0, 0, sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);
			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);

			cmdList.useProgram(program_horizontal.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur);
			cmdList.namedFramebufferTexture(fboBlur, GL_COLOR_ATTACHMENT0, sceneContext.ssaoMapTemp, 0);
			pathos::checkFramebufferStatus(cmdList, fboBlur, "fboBlur is invalid");
			cmdList.bindTextureUnit(0, sceneContext.ssaoMap);
			cmdList.bindTextureUnit(1, sceneContext.sceneDepth);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.useProgram(program_vertical.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur2);
			cmdList.namedFramebufferTexture(fboBlur2, GL_COLOR_ATTACHMENT0, sceneContext.ssaoMap, 0);
			pathos::checkFramebufferStatus(cmdList, fboBlur2, "fboBlur2 is invalid");
			cmdList.bindTextureUnit(0, sceneContext.ssaoMapTemp);
			cmdList.bindTextureUnit(1, sceneContext.sceneDepth);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		}
	}

}
