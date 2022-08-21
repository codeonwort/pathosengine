#include "screen_space_reflection.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/shader/shader_program.h"
#include "pathos/mesh/geometry.h"
#include "pathos/util/math_lib.h"

namespace pathos {

	class HiZBufferVS : public ShaderStage {
	public:
		HiZBufferVS() : ShaderStage(GL_VERTEX_SHADER, "HiZBufferVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class HiZBufferFS : public ShaderStage {
	public:
		HiZBufferFS() : ShaderStage(GL_FRAGMENT_SHADER, "HiZBufferFS") {
			setFilepath("hierarchical_z_buffer.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_HiZ, HiZBufferVS, HiZBufferFS);

}

namespace pathos {
	
	ScreenSpaceReflectionPass::ScreenSpaceReflectionPass() {}

	ScreenSpaceReflectionPass::~ScreenSpaceReflectionPass() {
		CHECK(destroyed);
	}

	void ScreenSpaceReflectionPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo_HiZ);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo_HiZ, -1, "FBO_SSR_HiZ");
	}

	void ScreenSpaceReflectionPass::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo_HiZ);
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

		// Generate HiZ
		{
			SCOPED_DRAW_EVENT(HiZ);

			// Copy sceneDepth to HiZ mip0
			cmdList.copyImageSubData(
				sceneContext.sceneDepth, GL_TEXTURE_2D, 0, 0, 0, 0,
				sceneContext.sceneDepthHiZ, GL_TEXTURE_2D, 0, 0, 0, 0,
				sceneContext.sceneWidth, sceneContext.sceneHeight, 1);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_HiZ);
			cmdList.useProgram(program.getGLName());

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_HiZ);
			cmdList.namedFramebufferDrawBuffers(fbo_HiZ, 0, nullptr);

			{
				cmdList.depthMask(GL_TRUE);
#if 0
				// Hmm... Depth write does not work if depth test is disabled?
				cmdList.disable(GL_DEPTH_TEST);
#else
				cmdList.enable(GL_DEPTH_TEST);
				cmdList.depthFunc(GL_ALWAYS);
#endif
				cmdList.disable(GL_BLEND);
			}

			constexpr GLenum PF_HiZ = GL_DEPTH_COMPONENT24;
			uint32 prevWidth = sceneContext.sceneWidth;
			uint32 prevHeight = sceneContext.sceneHeight;
			uint32 currentWidth, currentHeight;
			for (uint32 currentMip = 1; currentMip < sceneContext.sceneDepthHiZMipmapCount; ++currentMip) {
				currentWidth = pathos::max(1u, prevWidth >> 1);
				currentHeight = pathos::max(1u, prevHeight >> 1);

				cmdList.viewport(0, 0, currentWidth, currentHeight);

				cmdList.bindTextureUnit(0, sceneContext.sceneDepthHiZViews[currentMip - 1]);
				//cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepthHiZViews[currentMip], 0);
				cmdList.namedFramebufferTexture(fbo_HiZ, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepthHiZ, currentMip);

				fullscreenQuad->activate_position_uv(cmdList);
				fullscreenQuad->activateIndexBuffer(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);

				prevWidth = currentWidth;
				prevHeight = currentHeight;
			}

			{
				cmdList.namedFramebufferTexture(fbo_HiZ, GL_DEPTH_ATTACHMENT, 0, 0);
				cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			}
		}
	}

}
