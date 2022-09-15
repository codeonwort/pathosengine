#include "direct_lighting.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/shader/shader_program.h"
#include "pathos/camera/camera.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_enable_shadow("r.shadow", 1, "0 = disable shadowing, 1 = enable shadowing");

	// #todo-fog: Rework height fog
	static ConsoleVariable<int32> cvar_enable_fog("r.fog", 0, "0 = disable fog, 1 = enable fog");
	static ConsoleVariable<float> cvar_fog_bottom("r.fog.bottom", 0.0f, "bottom Y");
	static ConsoleVariable<float> cvar_fog_top("r.fog.top", 1000.0f, "top Y");
	static ConsoleVariable<float> cvar_fog_attenuation("r.fog.attenuation", 0.001f, "fog attenuation coefficient");

	struct UBO_DirectLighting {
		static const uint32 BINDING_SLOT = 1;

		vector4i enabledTechniques1; // (shadow, fog, ?, ?)
		vector4 fogColor;
		vector4 fogParams;           // (bottomY, topY, ?, ?)
	};

	class DirectLightingVS : public ShaderStage {
	public:
		DirectLightingVS() : ShaderStage(GL_VERTEX_SHADER, "DirectLightingVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class DirectLightingFS : public ShaderStage {
	public:
		DirectLightingFS() : ShaderStage(GL_FRAGMENT_SHADER, "DirectLightingFS")
		{
			setFilepath("direct_lighting.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_DirectLighting, DirectLightingVS, DirectLightingFS);

}

namespace pathos {

	DirectLightingPass::DirectLightingPass()
		: fbo(0xffffffff)
	{
	}

	DirectLightingPass::~DirectLightingPass() {
		CHECK(destroyed);
	}

	void DirectLightingPass::initializeResources(RenderCommandList& cmdList) {
		// fullscreen quad
		quad = new PlaneGeometry(2.0f, 2.0f);
		
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		ubo.init<UBO_DirectLighting>();
	}

	void DirectLightingPass::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
			quad->dispose();
			delete quad;
		}
		destroyed = true;
	}

	void DirectLightingPass::bindFramebuffer(RenderCommandList& cmdList) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		// #note-lighting: Clear sceneColor as direct lighting is first
		// and then indirect lighting comes. This may change in future.
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.clearBufferfv(GL_COLOR, 0, zero);
	}

	void DirectLightingPass::renderDirectLighting(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera)
	{
		SCOPED_DRAW_EVENT(DirectLighting);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_DirectLighting);
		cmdList.useProgram(program.getGLName());
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);

		pathos::checkFramebufferStatus(cmdList, fbo, "[DirectLighting] FBO is invalid");

		GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(5, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(6, sceneContext.cascadedShadowMap);
		cmdList.bindTextureUnit(7, sceneContext.omniShadowMaps);
		cmdList.bindTextureUnit(8, scene->irradianceMap);
		cmdList.bindTextureUnit(9, scene->prefilterEnvMap);
		cmdList.bindTextureUnit(10, IrradianceBaker::getBRDFIntegrationMap_512());

		cmdList.disable(GL_DEPTH_TEST);

		UBO_DirectLighting uboData;
		uboData.enabledTechniques1.x  = cvar_enable_shadow.getInt();
		uboData.enabledTechniques1.y  = cvar_enable_fog.getInt();
		uboData.fogColor              = glm::vec4(0.7f, 0.8f, 0.9f, 0.0f);
		uboData.fogParams.x           = cvar_fog_bottom.getFloat();
		uboData.fogParams.y           = cvar_fog_top.getFloat();
		uboData.fogParams.z           = cvar_fog_attenuation.getFloat();
		ubo.update(cmdList, UBO_DirectLighting::BINDING_SLOT, &uboData);

		quad->activate_position_uv(cmdList);
		quad->activateIndexBuffer(cmdList);
		quad->drawPrimitive(cmdList);

		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

}
