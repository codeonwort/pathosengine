#include "deferredpass_unpack.h"
#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/shader/shader_program.h"
#include "pathos/render/render_device.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	static ConsoleVariable<int32_t> cvar_enable_shadow("r.shadow", 1, "0 = disable shadow, 1 = enable shadow");

	static ConsoleVariable<int32_t> cvar_enable_fog("r.fog", 0, "0 = disable fog, 1 = enable fog");
	static ConsoleVariable<float> cvar_fog_bottom("r.fog.bottom", 0.0f, "bottom Y");
	static ConsoleVariable<float> cvar_fog_top("r.fog.top", 1000.0f, "top Y");
	static ConsoleVariable<float> cvar_fog_attenuation("r.fog.attenuation", 0.001f, "fog attenuation coefficient");

	struct UBO_Unpack {
		glm::ivec4 enabledTechniques1; // (shadow, fog, ?, ?)
		glm::vec4 fogColor;
		glm::vec4 fogParams;           // (bottomY, topY, ?, ?)
		float prefilterEnvMapMaxLOD;
	};

	class UnpackGBufferVS : public ShaderStage {
	public:
		UnpackGBufferVS() : ShaderStage(GL_VERTEX_SHADER, "UnpackGBufferVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class UnpackGBufferFS : public ShaderStage {
	public:
		UnpackGBufferFS() : ShaderStage(GL_FRAGMENT_SHADER, "UnpackGBufferFS")
		{
			setFilepath("deferred_unpack.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_UnpackGBuffer, UnpackGBufferVS, UnpackGBufferFS);

}

namespace pathos {

	MeshDeferredRenderPass_Unpack::MeshDeferredRenderPass_Unpack()
		: fbo(0xffffffff)
	{
	}

	MeshDeferredRenderPass_Unpack::~MeshDeferredRenderPass_Unpack() {
		CHECK(destroyed);
	}

	void MeshDeferredRenderPass_Unpack::initializeResources(RenderCommandList& cmdList) {
		// fullscreen quad
		quad = new PlaneGeometry(2.0f, 2.0f);
		createResource(cmdList);

		ubo_unpack.init<UBO_Unpack>();
	}

	void MeshDeferredRenderPass_Unpack::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
			quad->dispose();
			delete quad;
		}
		destroyed = true;
	}

	void MeshDeferredRenderPass_Unpack::createResource(RenderCommandList& cmdList) {
		//auto checkFramebufferStatus = [&cmdList](GLuint fbo) -> void {
		//	GLenum completeness;
		//	cmdList.checkNamedFramebufferStatus(fbo, GL_DRAW_FRAMEBUFFER, &completeness);
		//	cmdList.flushAllCommands(); // #todo-cmd-list: Don't flush here
		//	if (completeness != GL_FRAMEBUFFER_COMPLETE) {
		//		LOG(LogFatal, "%s: Failed to initialize fbo", __FUNCTION__);
		//		CHECK(0);
		//	}
		//};

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		//checkFramebufferStatus(fbo); // #todo-framebuffer: Can't check completeness now
	}

	void MeshDeferredRenderPass_Unpack::bindFramebuffer(RenderCommandList& cmdList) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.clearBufferfv(GL_COLOR, 0, zero);
	}

	void MeshDeferredRenderPass_Unpack::render(RenderCommandList& cmdList, Scene* scene, Camera* camera) {
		SCOPED_DRAW_EVENT(UnpackHDR);
		
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_UnpackGBuffer);
		cmdList.useProgram(program.getGLName());
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);

		GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(5, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(6, sceneContext.cascadedShadowMap);
		cmdList.bindTextureUnit(7, scene->irradianceMap);
		cmdList.bindTextureUnit(8, scene->prefilterEnvMap);
		cmdList.bindTextureUnit(9, IrradianceBaker::getBRDFIntegrationMap_512());

		cmdList.disable(GL_DEPTH_TEST);

		UBO_Unpack uboData;
		uboData.enabledTechniques1.x  = cvar_enable_shadow.getInt();
		uboData.enabledTechniques1.y  = cvar_enable_fog.getInt();
		uboData.fogColor              = glm::vec4(0.7f, 0.8f, 0.9f, 0.0f);
		uboData.fogParams.x           = cvar_fog_bottom.getFloat();
		uboData.fogParams.y           = cvar_fog_top.getFloat();
		uboData.fogParams.z           = cvar_fog_attenuation.getFloat();
		uboData.prefilterEnvMapMaxLOD = pathos::max(0.0f, (float)(scene->prefilterEnvMapMipLevels - 1));
		ubo_unpack.update(cmdList, 1, &uboData);

		quad->activate_position_uv(cmdList);
		quad->activateIndexBuffer(cmdList);
		quad->drawPrimitive(cmdList);

		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

}
