#include "screen_space_reflection.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/shader/shader_program.h"
#include "pathos/mesh/geometry.h"
#include "pathos/util/math_lib.h"

namespace pathos {

	// #todo-ssr: Unify every fullscreen vertex shaders
	class SSRFullscreenVS : public ShaderStage {
	public:
		SSRFullscreenVS() : ShaderStage(GL_VERTEX_SHADER, "SSRFullscreenVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class HiZBufferInitFS : public ShaderStage {
	public:
		HiZBufferInitFS() : ShaderStage(GL_FRAGMENT_SHADER, "HiZBufferInitFS") {
			addDefine("FIRST_MIP 1");
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
			setFilepath("ssr_preintegration.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_SSR_Preintegration, SSRFullscreenVS, PreintegrationFS);

}

namespace pathos {
	
	ScreenSpaceReflectionPass::ScreenSpaceReflectionPass() {}

	ScreenSpaceReflectionPass::~ScreenSpaceReflectionPass() {
		CHECK(destroyed);
	}

	void ScreenSpaceReflectionPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo_HiZ);
		gRenderDevice->createFramebuffers(1, &fbo_preintegration);

		cmdList.objectLabel(GL_FRAMEBUFFER, fbo_HiZ, -1, "FBO_SSR_HiZ");
		cmdList.namedFramebufferDrawBuffer(fbo_HiZ, GL_COLOR_ATTACHMENT0);

		cmdList.objectLabel(GL_FRAMEBUFFER, fbo_preintegration, -1, "FBO_SSR_Preintegration");
		cmdList.namedFramebufferDrawBuffer(fbo_preintegration, GL_COLOR_ATTACHMENT0);
	}

	void ScreenSpaceReflectionPass::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo_HiZ);
			gRenderDevice->deleteFramebuffers(1, &fbo_preintegration);
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

					cmdList.bindTextureUnit(0, sceneContext.sceneDepthHiZViews[currentMip - 1]);
					cmdList.namedFramebufferTexture(fbo_HiZ, GL_COLOR_ATTACHMENT0, sceneContext.sceneDepthHiZ, currentMip);

					fullscreenQuad->activate_position_uv(cmdList);
					fullscreenQuad->activateIndexBuffer(cmdList);
					fullscreenQuad->drawPrimitive(cmdList);

					prevWidth = currentWidth;
					prevHeight = currentHeight;
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

				cmdList.bindTextureUnit(0, sceneContext.ssrPreintegrationViews[currentMip - 1]);
				cmdList.bindTextureUnit(1, sceneContext.sceneDepthHiZViews[currentMip - 1]);
				cmdList.bindTextureUnit(2, sceneContext.sceneDepthHiZViews[currentMip]);
				cmdList.namedFramebufferTexture(fbo_preintegration, GL_COLOR_ATTACHMENT0, sceneContext.ssrPreintegration, currentMip);

				fullscreenQuad->activate_position_uv(cmdList);
				fullscreenQuad->activateIndexBuffer(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);

				prevWidth = currentWidth;
				prevHeight = currentHeight;
			}

			cmdList.namedFramebufferTexture(fbo_preintegration, GL_COLOR_ATTACHMENT0, 0, 0);
		}

		// 3. Ray-Tracing Pass
		{
			//
		}

		// 4. Pre-convolution Pass
		{
			//
		}

		// 5. Cone-Tracing Pass
		{
			//
		}
	}

}
