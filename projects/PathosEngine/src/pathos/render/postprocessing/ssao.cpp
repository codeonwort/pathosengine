#include "ssao.h"
#include "pathos/console.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/engine_util.h"

#include "badger/math/random.h"
#include "badger/math/minmax.h"

// Based on "Robust Screen Space Ambient Occlusion in 1 ms in 1080p on PS4"
//                                               (Wojciech Sterna, GPU Zen)
// It's said that this impl. was used in <Shadow Warrior> and <Shadow Warrior 2>

// Algorithm overview
// [v] 1. Compute SSAO in a quater-resolution buffer
// [ ] 2. Blur the SSAO output in two depth-aware, separable passes.
//     -> Not depth-aware yet
// [ ] 3. Upsample the blurred SSAO with a bilateral filter.
//     -> Not bilateral upsample yet

// #todo-postprocess-ssao: SSAO is really noisy. Check 'r.viewmode 7'

// Screen Space Ambient Occlusion
// https://learnopengl.com/Advanced-Lighting/SSAO

namespace pathos {

	static constexpr GLuint UBO_SSAO_BINDING_POINT = 1;
	static constexpr GLuint UBO_SSAO_RANDOM_BINDING_POINT = 2;

	static ConsoleVariable<float> cvar_ssao_radius("r.ssao.radius", 0.5f, "Radius of sample space");
	static ConsoleVariable<int32> cvar_ssao_enable("r.ssao.enable", 1, "Enable SSAO");
	static ConsoleVariable<int32> cvar_ssao_spp("r.ssao.samplesPerPixel", 32, "Determines SPP");
	static ConsoleVariable<int32> cvar_ssao_randomize_points("r.ssao.randomizePoints", 1, "Randomize sample points");

	struct UBO_SSAO {
		float ssaoRadius;
		uint32 enable;
		uint32 spp;             // samples per pixel
		uint32 randomizePoints; // bool in shader
	};

	class SSAO_Compute : public ShaderStage {
		
	public:
		SSAO_Compute()
			: ShaderStage(GL_COMPUTE_SHADER, "SSAO_Compute")
		{
			addDefine("SSAO_MAX_SAMPLE_POINTS", SSAO_MAX_SAMPLE_POINTS);
			addDefine("SSAO_NUM_ROTATION_NOISE", SSAO_NUM_ROTATION_NOISE);
			setFilepath("ssao_ao.glsl");
		}

	};

	DEFINE_COMPUTE_PROGRAM(Program_SSAO_Compute, SSAO_Compute);

}

namespace pathos {

	void SSAO::initializeResources(RenderCommandList& cmdList)
	{
		{
			Shader cs_downscale(GL_COMPUTE_SHADER, "CS_SSAO_Downscale");
			cs_downscale.loadSource("ssao_downscale.glsl");
			program_downscale = pathos::createProgram(cs_downscale, "SSAO_Downscale");
		}
		{
			Shader vs_blur(GL_VERTEX_SHADER, "VS_SSAO_BLUR_1");
			Shader fs_blur(GL_FRAGMENT_SHADER, "FS_SSAO_BLUR_1");
			fs_blur.addDefine("HORIZONTAL 1");
			vs_blur.loadSource("fullscreen_quad.glsl");
			fs_blur.loadSource("two_pass_gaussian_blur.glsl");
			program_blur = pathos::createProgram(vs_blur, fs_blur, "SSAO_Blur_1");
		}
		{
			Shader vs_blur(GL_VERTEX_SHADER, "VS_SSAO_BLUR_2");
			Shader fs_blur(GL_FRAGMENT_SHADER, "FS_SSAO_BLUR_2");
			fs_blur.addDefine("HORIZONTAL 0");
			vs_blur.loadSource("fullscreen_quad.glsl");
			fs_blur.loadSource("two_pass_gaussian_blur.glsl");
			program_blur2 = pathos::createProgram(vs_blur, fs_blur, "SSAO_Blur_2");
		}

		ubo.init<UBO_SSAO>();
		uboRandom.init<UBO_SSAO_Random>();

		gRenderDevice->createFramebuffers(1, &fboBlur);
		cmdList.namedFramebufferDrawBuffer(fboBlur, GL_COLOR_ATTACHMENT0);

		gRenderDevice->createFramebuffers(1, &fboBlur2);
		cmdList.namedFramebufferDrawBuffer(fboBlur2, GL_COLOR_ATTACHMENT0);
	}

	void SSAO::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteProgram(program_downscale);
		gRenderDevice->deleteProgram(program_blur);
		gRenderDevice->deleteProgram(program_blur2);
		gRenderDevice->deleteFramebuffers(1, &fboBlur);
		gRenderDevice->deleteFramebuffers(1, &fboBlur2);

		markDestroyed();
	}

	void SSAO::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(SSAO);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		{
			SCOPED_DRAW_EVENT(SSAODownsample);

			GLuint workGroupsX = (GLuint)ceilf((float)(sceneContext.sceneWidth / 2) / 64.0f);

			cmdList.useProgram(program_downscale);
			
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
			uboData.ssaoRadius      = cvar_ssao_radius.getFloat();
			uboData.enable          = cvar_ssao_enable.getInt() == 0 ? 0 : 1;
			uboData.spp             = badger::clamp(1u, (uint32)cvar_ssao_spp.getInt(), SSAO_MAX_SAMPLE_POINTS);
			uboData.randomizePoints = (cvar_ssao_randomize_points.getInt() == 0) ? 0 : 1;
			ubo.update(cmdList, UBO_SSAO_BINDING_POINT, &uboData);

			if (bRandomDataValid == false) {
				for (uint32 i = 0; i < SSAO_MAX_SAMPLE_POINTS; ++i) {
					// Sample kernel: random points inside unit hemisphere around +z axis
					vector3 p = RandomInUnitSphere();
					if (p.z < 0.0f) p.z = -p.z;

					// Put more samples closer to the origin
					float weight = (float)i / SSAO_MAX_SAMPLE_POINTS;
					weight = pathos::lerp(0.5f, 1.0f, weight * weight);
					p *= weight;

					randomData.samplePoints[i] = vector4(p.x, p.y, p.z, 0.0f);
				}
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

			cmdList.viewport(0, 0, sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);
			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);

			cmdList.useProgram(program_blur);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur);
			cmdList.namedFramebufferTexture(fboBlur, GL_COLOR_ATTACHMENT0, sceneContext.ssaoMapTemp, 0);
			pathos::checkFramebufferStatus(cmdList, fboBlur, "fboBlur is invalid");
			cmdList.bindTextureUnit(0, sceneContext.ssaoMap);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.useProgram(program_blur2);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur2);
			cmdList.namedFramebufferTexture(fboBlur2, GL_COLOR_ATTACHMENT0, sceneContext.ssaoMap, 0);
			pathos::checkFramebufferStatus(cmdList, fboBlur2, "fboBlur2 is invalid");
			cmdList.bindTextureUnit(0, sceneContext.ssaoMapTemp);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		}
	}

}
