#include "resolve_unlit.h"

#include "pathos/engine.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/mesh/geometry.h"
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	class ResolveUnlitVS : public ShaderStage {
	public:
		ResolveUnlitVS() : ShaderStage(GL_VERTEX_SHADER, "ResolveUnlitVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class ResolveUnlitFS : public ShaderStage {
	public:
		ResolveUnlitFS() : ShaderStage(GL_FRAGMENT_SHADER, "ResolveUnlitFS") {
			setFilepath("resolve_unlit.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_ResolveUnlit, ResolveUnlitVS, ResolveUnlitFS);

}

namespace pathos {

	ResolveUnlitPass::ResolveUnlitPass() {}

	ResolveUnlitPass::~ResolveUnlitPass() {
		CHECK(destroyed);
	}

	void ResolveUnlitPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
	}

	void ResolveUnlitPass::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		destroyed = true;
	}

	void ResolveUnlitPass::renderUnlit(
		RenderCommandList& cmdList,
		MeshGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(ResolveUnlit);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_ResolveUnlit);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);

		pathos::checkFramebufferStatus(cmdList, fbo, "[ResolveUnlit] FBO is invalid");

		// Set render states
		{
			cmdList.disable(GL_DEPTH_TEST);

			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		}

		GLuint* gbuffer_textures = (GLuint*)cmdList.allocateSingleFrameMemory(3 * sizeof(GLuint));
		gbuffer_textures[0] = sceneContext.gbufferA;
		gbuffer_textures[1] = sceneContext.gbufferB;
		gbuffer_textures[2] = sceneContext.gbufferC;

		cmdList.bindTextures(0, 3, gbuffer_textures);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		cmdList.bindTextures(0, 3, nullptr);

		// Restore render states
		{
			cmdList.disable(GL_BLEND);

			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

}
