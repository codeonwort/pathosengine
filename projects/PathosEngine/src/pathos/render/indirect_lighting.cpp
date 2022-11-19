#include "indirect_lighting.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/shader/shader_program.h"
#include "pathos/scene/camera.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	static ConsoleVariable<float> cvar_gi_intensity("r.probegi.intensity", 1.0f, "Indirect lighting boost coeff");

	struct UBO_IndirectLighting {
		static const uint32 BINDING_SLOT = 1;

		float prefilterEnvMapMaxLOD;
		float intensity;
	};

	class IndirectLightingVS : public ShaderStage {
	public:
		IndirectLightingVS() : ShaderStage(GL_VERTEX_SHADER, "IndirectLightingVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class IndirectLightingFS : public ShaderStage {
	public:
		IndirectLightingFS() : ShaderStage(GL_FRAGMENT_SHADER, "IndirectLightingFS") {
			setFilepath("indirect_lighting.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_IndirectLighting, IndirectLightingVS, IndirectLightingFS);

}

namespace pathos {

	IndirectLightingPass::IndirectLightingPass() {}

	IndirectLightingPass::~IndirectLightingPass() {
		CHECK(destroyed);
	}

	void IndirectLightingPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		ubo.init<UBO_IndirectLighting>();
	}

	void IndirectLightingPass::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		destroyed = true;
	}

	void IndirectLightingPass::renderIndirectLighting(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(IndirectLighting);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_IndirectLighting);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);

		pathos::checkFramebufferStatus(cmdList, fbo, "[IndirectLighting] FBO is invalid");

		// Set render states
		{
			cmdList.disable(GL_DEPTH_TEST);

			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		}

		UBO_IndirectLighting uboData{};
		uboData.prefilterEnvMapMaxLOD = std::max(0.0f, (float)(scene->prefilterEnvMapMipLevels - 1));
		uboData.intensity = std::max(0.0f, cvar_gi_intensity.getFloat());
		ubo.update(cmdList, UBO_IndirectLighting::BINDING_SLOT, &uboData);

		GLuint* gbuffer_textures = (GLuint*)cmdList.allocateSingleFrameMemory(3 * sizeof(GLuint));
		gbuffer_textures[0] = sceneContext.gbufferA;
		gbuffer_textures[1] = sceneContext.gbufferB;
		gbuffer_textures[2] = sceneContext.gbufferC;

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(3, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(4, scene->irradianceMap);
		cmdList.bindTextureUnit(5, scene->prefilterEnvMap);
		cmdList.bindTextureUnit(6, IrradianceBaker::getBRDFIntegrationMap_512());

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		// Fix a strange bug that IBL maps are randomly persistent across worlds.
		cmdList.bindTextures(0, 7, nullptr);

		// Restore render states
		{
			cmdList.disable(GL_BLEND);

			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

}
