#include "bloom.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"
#include "pathos/engine.h"
#include "pathos/util/engine_util.h"

#include "badger/math/minmax.h"

namespace pathos {

	class BloomVS : public ShaderStage {
	public:
		BloomVS() : ShaderStage(GL_VERTEX_SHADER, "BloomVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class BloomDownsampleFS : public ShaderStage {
	public:
		BloomDownsampleFS() : ShaderStage(GL_FRAGMENT_SHADER, "BloomDownsampleFS") {
			setFilepath("bloom_downsample.glsl");
		}
	};

	struct UBO_BloomUpsample {
		static constexpr uint32 BINDING_POINT = 1;

		float filterRadius;
	};

	class BloomUpsampleFS : public ShaderStage {
	public:
		BloomUpsampleFS() : ShaderStage(GL_FRAGMENT_SHADER, "BloomUpsampleFS") {
			setFilepath("bloom_upsample.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_BloomDownsample, BloomVS, BloomDownsampleFS);
	DEFINE_SHADER_PROGRAM2(Program_BloomUpsample, BloomVS, BloomUpsampleFS);

}

namespace pathos {

	void BloomPass::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_BloomPass");

		uboUpsample.init<UBO_BloomUpsample>("UBO_BloomUpsample");
	}

	void BloomPass::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void BloomPass::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(BloomPass);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint input0 = getInput(EPostProcessInput::PPI_0); // Source for bloom chain mip0

		const uint32 bloomChainMipCount = sceneContext.sceneBloomChainMipCount;
		std::vector<GLuint>& bloomChainViews = sceneContext.sceneBloomChainViews;

		{
			SCOPED_DRAW_EVENT(Downsample);

			const ShaderProgram& program = FIND_SHADER_PROGRAM(Program_BloomDownsample);
			cmdList.useProgram(program.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

			uint32 viewportWidth = sceneContext.sceneWidth / 2;
			uint32 viewportHeight = sceneContext.sceneHeight / 2;
			for (uint32 mipIx = 0; mipIx < bloomChainMipCount; ++mipIx) {
				GLuint srcTexture = (mipIx == 0) ? input0 : bloomChainViews[mipIx - 1];
				GLuint dstTexture = bloomChainViews[mipIx];

				cmdList.viewport(0, 0, viewportWidth, viewportHeight);

				cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, dstTexture, 0);
				cmdList.bindTextureUnit(0, srcTexture);

				fullscreenQuad->activate_position_uv(cmdList);
				fullscreenQuad->activateIndexBuffer(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);

				viewportWidth /= 2;
				viewportHeight /= 2;
			}
		}

		{
			SCOPED_DRAW_EVENT(Upsample);

			cmdList.enable(GL_BLEND);
			cmdList.blendFunc(GL_ONE, GL_ONE);

			const ShaderProgram& program = FIND_SHADER_PROGRAM(Program_BloomUpsample);
			cmdList.useProgram(program.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

			uint32 mip0Width = sceneContext.sceneWidth / 2;
			uint32 mip0Height = sceneContext.sceneHeight / 2;
			for (int32 mipIx = bloomChainMipCount - 1; mipIx > 0; --mipIx) {
				GLuint srcTexture = bloomChainViews[mipIx];
				GLuint dstTexture = bloomChainViews[mipIx - 1];

				cmdList.viewport(0, 0, mip0Width >> (mipIx - 1), mip0Height >> (mipIx - 1));

				UBO_BloomUpsample uboData;
				uboData.filterRadius = 1.0f;
				uboUpsample.update(cmdList, UBO_BloomUpsample::BINDING_POINT, &uboData);

				cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, dstTexture, 0);
				cmdList.bindTextureUnit(0, srcTexture);

				fullscreenQuad->activate_position_uv(cmdList);
				fullscreenQuad->activateIndexBuffer(cmdList);
				fullscreenQuad->drawPrimitive(cmdList);
			}

			cmdList.disable(GL_BLEND);
		}
	}

}
