#include "ssao.h"
#include "pathos/console.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/scene_render_targets.h"

#include "badger/math/random.h"

namespace pathos {

	static constexpr GLuint UBO_SSAO_BINDING_POINT = 1;
	static constexpr GLuint UBO_SSAO_RANDOM_BINDING_POINT = 2;

	static ConsoleVariable<float> cvar_ssao_radius("r.ssao.radius", 0.1f, "Radius of sample space");
	static ConsoleVariable<int32> cvar_ssao_enable("r.ssao.enable", 1, "Enable SSAO");
	static ConsoleVariable<int32> cvar_ssao_point_count("r.ssao.pointCount", 32, "Determines sample count for occlusion calculation");
	static ConsoleVariable<int32> cvar_ssao_randomize_points("r.ssao.randomizePoints", 1, "Randomize sample points");

	struct UBO_SSAO {
		float ssaoRadius;
		uint32 enable;
		uint32 pointCount;
		uint32 randomizePoints; // bool in shader
	};

	class SSAO_Compute : public ShaderStage {
		
	public:
		SSAO_Compute()
			: ShaderStage(GL_COMPUTE_SHADER, "SSAO_Compute")
		{
			setFilepath("ssao_ao.glsl");
		}

	};

	DEFINE_COMPUTE_PROGRAM(Program_SSAO_Compute, SSAO_Compute);

}

namespace pathos {

	void SSAO::initializeResources(RenderCommandList& cmdList)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

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

		cmdList.createFramebuffers(1, &fboBlur);
		cmdList.namedFramebufferTexture(fboBlur, GL_COLOR_ATTACHMENT0, sceneContext.ssaoMapTemp, 0);
		cmdList.namedFramebufferDrawBuffer(fboBlur, GL_COLOR_ATTACHMENT0);
		checkFramebufferStatus(cmdList, fboBlur);

		cmdList.createFramebuffers(1, &fboBlur2);
		cmdList.namedFramebufferTexture(fboBlur2, GL_COLOR_ATTACHMENT0, sceneContext.ssaoMap, 0);
		cmdList.namedFramebufferDrawBuffer(fboBlur2, GL_COLOR_ATTACHMENT0);
		checkFramebufferStatus(cmdList, fboBlur2);
	}

	void SSAO::releaseResources(RenderCommandList& cmdList)
	{
		cmdList.deleteProgram(program_downscale);
		cmdList.deleteProgram(program_blur);
		cmdList.deleteProgram(program_blur2);
		cmdList.deleteFramebuffers(1, &fboBlur);
		cmdList.deleteFramebuffers(1, &fboBlur2);

		markDestroyed();
	}

	void SSAO::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(SSAO);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		{
			SCOPED_DRAW_EVENT(Downsample);

			GLuint workGroupsX = (GLuint)ceilf((float)(sceneContext.sceneWidth / 2) / 64.0f);

			cmdList.useProgram(program_downscale);

			cmdList.bindImageTexture(0, sceneContext.gbufferA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32UI);
			cmdList.bindImageTexture(1, sceneContext.gbufferB, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			cmdList.bindImageTexture(2, sceneContext.ssaoHalfNormalAndDepth, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			cmdList.dispatchCompute(workGroupsX, sceneContext.sceneHeight / 2, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		{
			SCOPED_DRAW_EVENT(AO);

			GLuint workGroupsX = (GLuint)ceilf((float)(sceneContext.sceneWidth / 2) / 16.0f);
			GLuint workGroupsY = (GLuint)ceilf((float)(sceneContext.sceneHeight / 2) / 16.0f);

			ShaderProgram& program_computeAO = FIND_SHADER_PROGRAM(Program_SSAO_Compute);
			cmdList.useProgram(program_computeAO.getGLName());

			UBO_SSAO uboData;
			uboData.ssaoRadius      = cvar_ssao_radius.getFloat();
			uboData.enable          = cvar_ssao_enable.getInt() == 0 ? 0 : 1;
			uboData.pointCount      = (uint32)cvar_ssao_point_count.getInt();
			uboData.randomizePoints = cvar_ssao_randomize_points.getInt() == 0 ? 0 : 1;
			ubo.update(cmdList, UBO_SSAO_BINDING_POINT, &uboData);

			if (randomGenerated == false) {
				for (uint32 i = 0; i < 256; i++) {
					glm::vec3 p = RandomInUnitSphere();
					glm::vec3 v = RandomInUnitSphere();
					randomData.points[i] = glm::vec4(Random() * 2.0f - 1.0f, Random() * 2.0f - 1.0f, Random(), 0.0f);
					randomData.randomVectors[i] = glm::vec4(Random(), Random(), Random(), Random());
				}
				randomGenerated = true;
			}
			uboRandom.update(cmdList, UBO_SSAO_RANDOM_BINDING_POINT, &randomData);

			cmdList.bindImageTexture(0, sceneContext.ssaoHalfNormalAndDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
			cmdList.bindImageTexture(1, sceneContext.ssaoMap, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
			cmdList.dispatchCompute(workGroupsX, workGroupsY, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		{
			SCOPED_DRAW_EVENT(Blur);

			cmdList.useProgram(program_blur);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur);
			cmdList.bindTextureUnit(0, sceneContext.ssaoMap);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.useProgram(program_blur2);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur2);
			cmdList.bindTextureUnit(0, sceneContext.ssaoMapTemp);
			fullscreenQuad->drawPrimitive(cmdList);
		}
	}

}
