#include "ssao.h"
#include "pathos/shader/shader.h"
#include "pathos/render/scene_render_targets.h"

namespace pathos {

	void SSAO::initializeResources(RenderCommandList& cmdList)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		Shader cs_downscale(GL_COMPUTE_SHADER, "CS_SSAO_Downscale");
		cs_downscale.loadSource("ssao_downscale.glsl");

		program_downscale = pathos::createProgram(cs_downscale, "SSAO_Downscale");
	}

	void SSAO::releaseResources(RenderCommandList& cmdList)
	{
		cmdList.deleteProgram(program_downscale);

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

			// Wait, this is illegal. A depth texture cannot be bound as image2D.
			//cmdList.bindImageTexture(0, sceneContext.sceneDepth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
			cmdList.bindTextureUnit(0, sceneContext.sceneDepth);

			cmdList.bindImageTexture(1, sceneContext.gbufferA, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32UI);
			cmdList.bindImageTexture(2, sceneContext.ssaoHalfNormalAndDepth, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			cmdList.dispatchCompute(workGroupsX, sceneContext.sceneHeight / 2, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
	}

}
