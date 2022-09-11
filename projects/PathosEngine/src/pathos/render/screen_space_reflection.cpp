#include "screen_space_reflection.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/shader/shader_program.h"
#include "pathos/mesh/geometry.h"
#include "pathos/util/math_lib.h"
#include "pathos/engine_policy.h"

namespace pathos {

	// #todo-ssr: Unify every fullscreen vertex shaders
	class SSRFullscreenVS : public ShaderStage {
	public:
		SSRFullscreenVS() : ShaderStage(GL_VERTEX_SHADER, "SSRFullscreenVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	struct UBO_HiZ {
		static constexpr uint32 BINDING_POINT = 1;

		// #todo-math: ivector2
		uint32 prevSizeX;
		uint32 prevSizeY;
		uint32 currentSizeX;
		uint32 currentSizeY;
		uint32 needsExtraSampleX;
		uint32 needsExtraSampleY;
	};

	class HiZBufferInitFS : public ShaderStage {
	public:
		HiZBufferInitFS() : ShaderStage(GL_FRAGMENT_SHADER, "HiZBufferInitFS") {
			addDefine("FIRST_MIP 1");
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z 1");
			}
			setFilepath("hierarchical_z_buffer.glsl");
		}
	};
	class HiZBufferDownsampleFS : public ShaderStage {
	public:
		HiZBufferDownsampleFS() : ShaderStage(GL_FRAGMENT_SHADER, "HiZBufferDownsampleFS") {
			addDefine("FIRST_MIP 0");
			setFilepath("hierarchical_z_buffer.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_HiZ_Init, SSRFullscreenVS, HiZBufferInitFS);
	DEFINE_SHADER_PROGRAM2(Program_HiZ_Downsample, SSRFullscreenVS, HiZBufferDownsampleFS);

	// Calculates the scene visibility input
	// for the cone-tracing pass in a hierarchical fashion.
	class PreintegrationFS : public ShaderStage {
	public:
		PreintegrationFS() : ShaderStage(GL_FRAGMENT_SHADER, "PreintegrationFS") {
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z 1");
			}
			setFilepath("ssr_preintegration.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_SSR_Preintegration, SSRFullscreenVS, PreintegrationFS);

	struct UBO_ScreenSpaceRayTracing {
		static constexpr uint32 BINDING_POINT = 1;

		vector2 sceneSize;
		uint32 hiZMipCount;
	};

	class ScreenSpaceRayTracingFS : public ShaderStage {
	public:
		ScreenSpaceRayTracingFS() : ShaderStage(GL_FRAGMENT_SHADER, "ScreenSpaceRayTracingFS") {
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z 1");
			}
			setFilepath("ssr_raytracing.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_SSR_RayTracing, SSRFullscreenVS, ScreenSpaceRayTracingFS);

	class SSRPreconvolutionInitFS : public ShaderStage {
	public:
		SSRPreconvolutionInitFS() : ShaderStage(GL_FRAGMENT_SHADER, "SSRPreconvolutionInitFS") {
			addDefine("FIRST_MIP", 1);
			setFilepath("ssr_preconvolution.glsl");
		}
	};
	class SSRPreconvolutionHorizontalFS : public ShaderStage {
	public:
		SSRPreconvolutionHorizontalFS() : ShaderStage(GL_FRAGMENT_SHADER, "SSRPreconvolutionHorizontalFS") {
			addDefine("FIRST_MIP", 0);
			addDefine("HORIZONTAL", 1);
			setFilepath("ssr_preconvolution.glsl");
		}
	};
	class SSRPreconvolutionVerticalFS : public ShaderStage {
	public:
		SSRPreconvolutionVerticalFS() : ShaderStage(GL_FRAGMENT_SHADER, "SSRPreconvolutionVerticalFS") {
			addDefine("FIRST_MIP", 0);
			addDefine("HORIZONTAL", 0);
			setFilepath("ssr_preconvolution.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_SSRPreconvolution_Init, SSRFullscreenVS, SSRPreconvolutionInitFS);
	DEFINE_SHADER_PROGRAM2(Program_SSRPreconvolution_Horizontal, SSRFullscreenVS, SSRPreconvolutionHorizontalFS);
	DEFINE_SHADER_PROGRAM2(Program_SSRPreconvolution_Vertical, SSRFullscreenVS, SSRPreconvolutionVerticalFS);
}

namespace pathos {
	
	ScreenSpaceReflectionPass::ScreenSpaceReflectionPass() {}

	ScreenSpaceReflectionPass::~ScreenSpaceReflectionPass() {
		CHECK(destroyed);
	}

	void ScreenSpaceReflectionPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo_HiZ);
		gRenderDevice->createFramebuffers(1, &fbo_preintegration);
		gRenderDevice->createFramebuffers(1, &fbo_raytracing);
		gRenderDevice->createFramebuffers(1, &fbo_preconvolution);

		cmdList.objectLabel(GL_FRAMEBUFFER, fbo_HiZ, -1, "FBO_SSR_HiZ");
		cmdList.namedFramebufferDrawBuffer(fbo_HiZ, GL_COLOR_ATTACHMENT0);

		cmdList.objectLabel(GL_FRAMEBUFFER, fbo_preintegration, -1, "FBO_SSR_Preintegration");
		cmdList.namedFramebufferDrawBuffer(fbo_preintegration, GL_COLOR_ATTACHMENT0);

		cmdList.objectLabel(GL_FRAMEBUFFER, fbo_raytracing, -1, "FBO_SSR_RayTracing");
		cmdList.namedFramebufferDrawBuffer(fbo_raytracing, GL_COLOR_ATTACHMENT0);

		cmdList.objectLabel(GL_FRAMEBUFFER, fbo_preconvolution, -1, "FBO_SSR_Preconvolution");
		cmdList.namedFramebufferDrawBuffer(fbo_preconvolution, GL_COLOR_ATTACHMENT0);
		
		GLfloat* borderColor = (GLfloat*)cmdList.allocateSingleFrameMemory(4 * sizeof(GLfloat));
		borderColor[0] = borderColor[1] = borderColor[2] = borderColor[3] = 0.0f;

		gRenderDevice->createSamplers(1, &pointSampler);
		cmdList.samplerParameteri(pointSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		cmdList.samplerParameteri(pointSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		cmdList.samplerParameterfv(pointSampler, GL_TEXTURE_BORDER_COLOR, borderColor);
		cmdList.samplerParameteri(pointSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		cmdList.samplerParameteri(pointSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.objectLabel(GL_SAMPLER, pointSampler, -1, "Sampler_pointClamped");

		uboHiZ.init<UBO_HiZ>();
		uboRayTracing.init<UBO_ScreenSpaceRayTracing>();
	}

	void ScreenSpaceReflectionPass::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo_HiZ);
			gRenderDevice->deleteFramebuffers(1, &fbo_preintegration);
			gRenderDevice->deleteFramebuffers(1, &fbo_raytracing);
			gRenderDevice->deleteFramebuffers(1, &fbo_preconvolution);
			gRenderDevice->deleteSamplers(1, &pointSampler);
		}
		destroyed = true;
	}

	void ScreenSpaceReflectionPass::renderScreenSpaceReflection(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(ScreenSpaceReflection);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		// 1. HiZ Pass
		{
			SCOPED_DRAW_EVENT(HiZ);

			{
				ShaderProgram& program_init = FIND_SHADER_PROGRAM(Program_HiZ_Init);
				cmdList.useProgram(program_init.getGLName());

				cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_HiZ);

				cmdList.depthMask(GL_FALSE);
				cmdList.disable(GL_DEPTH_TEST);
				cmdList.disable(GL_BLEND);

				cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

				cmdList.bindTextureUnit(0, sceneContext.sceneDepth);
				cmdList.namedFramebufferTexture(fbo_HiZ, GL_COLOR_ATTACHMENT0, sceneContext.sceneDepthHiZ, 0);

				fullscreenQuad->activate_position_uv(cmdList);
				fullscreenQuad->activateIndexBuffer(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);

				cmdList.bindTextureUnit(0, 0);
			}

			{
				ShaderProgram& program_downsample = FIND_SHADER_PROGRAM(Program_HiZ_Downsample);
				cmdList.useProgram(program_downsample.getGLName());

				uint32 prevWidth = sceneContext.sceneWidth;
				uint32 prevHeight = sceneContext.sceneHeight;
				uint32 currentWidth, currentHeight;
				for (uint32 currentMip = 1; currentMip < sceneContext.sceneDepthHiZMipmapCount; ++currentMip) {
					currentWidth = pathos::max(1u, prevWidth >> 1);
					currentHeight = pathos::max(1u, prevHeight >> 1);

					cmdList.viewport(0, 0, currentWidth, currentHeight);

					UBO_HiZ uboData;
					uboData.prevSizeX = prevWidth;
					uboData.prevSizeY = prevHeight;
					uboData.currentSizeX = currentWidth;
					uboData.currentSizeY = currentHeight;
					uboData.needsExtraSampleX = (currentWidth * 2) < prevWidth;
					uboData.needsExtraSampleY = (currentHeight * 2) < prevHeight;
					uboHiZ.update(cmdList, UBO_HiZ::BINDING_POINT, &uboData);

					cmdList.bindTextureUnit(0, sceneContext.sceneDepthHiZViews[currentMip - 1]);
					cmdList.bindSampler(0, pointSampler);
					// This is more convenient to insepct in RenderDoc.
					cmdList.namedFramebufferTexture(fbo_HiZ, GL_COLOR_ATTACHMENT0, sceneContext.sceneDepthHiZ, currentMip);
					//cmdList.namedFramebufferTexture(fbo_HiZ, GL_COLOR_ATTACHMENT0, sceneContext.sceneDepthHiZViews[currentMip], 0);

					fullscreenQuad->activate_position_uv(cmdList);
					fullscreenQuad->activateIndexBuffer(cmdList);
					fullscreenQuad->drawPrimitive(cmdList);

					prevWidth = currentWidth;
					prevHeight = currentHeight;
					cmdList.bindSampler(0, 0);
				}

				cmdList.namedFramebufferTexture(fbo_HiZ, GL_COLOR_ATTACHMENT0, 0, 0);
			}
		}

		// 2. Pre-integration Pass
		{
			SCOPED_DRAW_EVENT(Preintegration);

			static const float clearValue = 1.0f;
			cmdList.clearTexSubImage(sceneContext.ssrPreintegration, 0,
				0, 0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight, 1,
				GL_RED, GL_FLOAT, &clearValue);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_SSR_Preintegration);
			cmdList.useProgram(program.getGLName());

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_preintegration);
			
			uint32 prevWidth = sceneContext.sceneWidth;
			uint32 prevHeight = sceneContext.sceneHeight;
			uint32 currentWidth, currentHeight;
			for (uint32 currentMip = 1; currentMip < sceneContext.ssrPreintegrationMipmapCount; ++currentMip) {
				currentWidth = pathos::max(1u, prevWidth >> 1);
				currentHeight = pathos::max(1u, prevHeight >> 1);

				cmdList.viewport(0, 0, currentWidth, currentHeight);

				const GLuint pointSamplers3[] = { pointSampler, pointSampler, pointSampler };
				cmdList.bindTextureUnit(0, sceneContext.ssrPreintegrationViews[currentMip - 1]);
				cmdList.bindTextureUnit(1, sceneContext.sceneDepthHiZViews[currentMip - 1]);
				cmdList.bindTextureUnit(2, sceneContext.sceneDepthHiZViews[currentMip]);
				cmdList.bindSamplers(0, 3, pointSamplers3);
				cmdList.namedFramebufferTexture(fbo_preintegration, GL_COLOR_ATTACHMENT0, sceneContext.ssrPreintegration, currentMip);

				fullscreenQuad->activate_position_uv(cmdList);
				fullscreenQuad->activateIndexBuffer(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);

				prevWidth = currentWidth;
				prevHeight = currentHeight;
				cmdList.bindSamplers(0, 3, nullptr);
			}

			cmdList.namedFramebufferTexture(fbo_preintegration, GL_COLOR_ATTACHMENT0, 0, 0);
		}

		// 3. Ray-Tracing Pass
		{
			SCOPED_DRAW_EVENT(ScreenSpaceRayTracing);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_SSR_RayTracing);
			cmdList.useProgram(program.getGLName());

			UBO_ScreenSpaceRayTracing uboData;
			uboData.sceneSize = vector2(sceneContext.sceneWidth, sceneContext.sceneHeight);
			uboData.hiZMipCount = sceneContext.sceneDepthHiZMipmapCount;
			uboRayTracing.update(cmdList, UBO_ScreenSpaceRayTracing::BINDING_POINT, &uboData);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_raytracing);
			cmdList.namedFramebufferTexture(fbo_raytracing, GL_COLOR_ATTACHMENT0, sceneContext.ssrRayTracing, 0);

			cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

			cmdList.bindTextureUnit(0, sceneContext.sceneColor);
			cmdList.bindTextureUnit(1, sceneContext.sceneDepth);
			cmdList.bindTextureUnit(2, sceneContext.sceneDepthHiZ);
			cmdList.bindTextureUnit(3, sceneContext.gbufferA);
			cmdList.bindTextureUnit(4, sceneContext.gbufferB);
			cmdList.bindTextureUnit(5, sceneContext.gbufferC);
			const GLuint pointSamplers6[] = {
				pointSampler, pointSampler, pointSampler,
				pointSampler, pointSampler, pointSampler };
			cmdList.bindSamplers(0, 6, pointSamplers6);

			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.bindSamplers(0, 6, nullptr);
		}

		// 4. Pre-convolution Pass
		{
			SCOPED_DRAW_EVENT(Preconvolution);

			// First mip size
			const uint32 preconvWidth = sceneContext.sceneWidth / 2;
			const uint32 preconvHeight = sceneContext.sceneHeight / 2;

			{
				const ShaderProgram& program = FIND_SHADER_PROGRAM(Program_SSRPreconvolution_Init);
				const GLuint fbo = fbo_preconvolution;

				cmdList.useProgram(program.getGLName());
				cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

				cmdList.depthMask(GL_FALSE);
				cmdList.disable(GL_DEPTH_TEST);
				cmdList.disable(GL_BLEND);

				cmdList.viewport(0, 0, preconvWidth, preconvHeight);

				cmdList.bindTextureUnit(0, sceneContext.sceneColor);
				cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.ssrPreconvolution, 0);

				fullscreenQuad->activate_position_uv(cmdList);
				fullscreenQuad->activateIndexBuffer(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);

				cmdList.bindTextureUnit(0, 0);
			}
			{
				const ShaderProgram& programH = FIND_SHADER_PROGRAM(Program_SSRPreconvolution_Horizontal);
				const ShaderProgram& programV = FIND_SHADER_PROGRAM(Program_SSRPreconvolution_Vertical);
				const GLuint fbo = fbo_preconvolution;

				uint32 prevWidth = preconvWidth;
				uint32 prevHeight = preconvHeight;
				uint32 currentWidth, currentHeight;
				for (uint32 currentMip = 1; currentMip < sceneContext.ssrPreconvolutionMipmapCount; ++currentMip) {
					currentWidth = std::max(1u, prevWidth >> 1);
					currentHeight = std::max(1u, prevHeight >> 1);

					cmdList.viewport(0, 0, currentWidth, currentHeight);

					cmdList.useProgram(programH.getGLName());
					cmdList.bindTextureUnit(0, sceneContext.ssrPreconvolutionViews[currentMip - 1]);
					cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.ssrPreconvolutionTemp, currentMip);
					fullscreenQuad->activate_position_uv(cmdList);
					fullscreenQuad->activateIndexBuffer(cmdList);
					fullscreenQuad->drawPrimitive(cmdList);

					cmdList.useProgram(programV.getGLName());
					cmdList.bindTextureUnit(0, sceneContext.ssrPreconvolutionTempViews[currentMip]);
					cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.ssrPreconvolution, currentMip);
					fullscreenQuad->activate_position_uv(cmdList);
					fullscreenQuad->activateIndexBuffer(cmdList);
					fullscreenQuad->drawPrimitive(cmdList);

					prevWidth = currentWidth;
					prevHeight = currentHeight;
				}
			}
		}

		// 5. Cone-Tracing Pass
		{
			//
		}
	}

}
