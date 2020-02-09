#include "deferredpass_unpack.h"
#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	struct UBO_Unpack {
		glm::ivec4 enabledTechniques1; // (shadow, fog, ?, ?)
		glm::vec4 fogColor;
		glm::vec4 fogParams;           // (bottomY, topY, ?, ?)
		glm::vec4 bloomParams;
	};

	static ConsoleVariable<int32_t> cvar_enable_shadow("r.shadow", 1, "0 = disable shadow, 1 = enable shadow");
	
	static ConsoleVariable<int32_t> cvar_enable_fog("r.fog", 0, "0 = disable fog, 1 = enable fog");
	static ConsoleVariable<float> cvar_fog_bottom("r.fog.bottom", 0.0f, "bottom Y");
	static ConsoleVariable<float> cvar_fog_top("r.fog.top", 1000.0f, "top Y");
	static ConsoleVariable<float> cvar_fog_attenuation("r.fog.attenuation", 0.001f, "fog attenuation coefficient");

	static ConsoleVariable<float> cvar_bloom_strength("r.bloom.strength", 4.0f, "Bloom strength");
	static ConsoleVariable<float> cvar_bloom_min("r.bloom.min", 0.8f, "Minimum bloom");
	static ConsoleVariable<float> cvar_bloom_max("r.bloom.max", 1.2f, "Maximum bloom");

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
		createProgram();
		createResource(cmdList);
	}

	void MeshDeferredRenderPass_Unpack::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			cmdList.deleteProgram(program);
			cmdList.deleteFramebuffers(1, &fbo);
			quad->dispose();
			delete quad;
		}
		destroyed = true;
	}

	void MeshDeferredRenderPass_Unpack::createProgram() {
		std::string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out VS_OUT {
	vec2 screenUV;
} vs_out;

void main() {
	gl_Position = vec4(position, 1.0);
	vs_out.screenUV = uv;
}
)";
		
		Shader vs(GL_VERTEX_SHADER, "VS_GBuffer_Unpack");
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER, "FS_GBuffer_Unpack");
		fs.loadSource("deferred_unpack.glsl");

		program = pathos::createProgram(vs, fs, "UnpackGBuffer");
		ubo_unpack.init<UBO_Unpack>();
	}

	void MeshDeferredRenderPass_Unpack::createResource(RenderCommandList& cmdList) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		auto checkFramebufferStatus = [&cmdList](GLuint fbo) -> void {
			GLenum completeness;
			cmdList.checkNamedFramebufferStatus(fbo, GL_DRAW_FRAMEBUFFER, &completeness);
			// #todo-cmd-list: Don't flush here
			cmdList.flushAllCommands();
			if (completeness != GL_FRAMEBUFFER_COMPLETE) {
				LOG(LogFatal, "%s: Failed to initialize fbo", __FUNCTION__);
				CHECK(0);
			}
		};

		// hdr resource
		GLenum hdr_draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		cmdList.createFramebuffers(1, &fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT1, sceneContext.sceneBloom, 0);
		cmdList.namedFramebufferDrawBuffers(fbo, 2, hdr_draw_buffers);
		checkFramebufferStatus(fbo);
	}

	void MeshDeferredRenderPass_Unpack::bindFramebuffer(RenderCommandList& cmdList) {
		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.clearBufferfv(GL_COLOR, 0, zero);
		cmdList.clearBufferfv(GL_COLOR, 1, zero);
		cmdList.clearBufferfv(GL_COLOR, 2, zero);
	}

	void MeshDeferredRenderPass_Unpack::render(RenderCommandList& cmdList, Scene* scene, Camera* camera) {
		SCOPED_DRAW_EVENT(UnpackHDR);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.useProgram(program);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(5, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(6, sceneContext.cascadedShadowMap);
		cmdList.bindTextureUnit(7, scene->irradianceMap);

		cmdList.disable(GL_DEPTH_TEST);

		UBO_Unpack uboData;
		uboData.enabledTechniques1.x = cvar_enable_shadow.getInt();
		uboData.enabledTechniques1.y = cvar_enable_fog.getInt();
		uboData.fogColor             = glm::vec4(0.7f, 0.8f, 0.9f, 0.0f);
		uboData.fogParams.x          = cvar_fog_bottom.getFloat();
		uboData.fogParams.y          = cvar_fog_top.getFloat();
		uboData.fogParams.z          = cvar_fog_attenuation.getFloat();
		uboData.bloomParams.x        = cvar_bloom_strength.getFloat();
		uboData.bloomParams.y        = cvar_bloom_min.getFloat();
		uboData.bloomParams.z        = cvar_bloom_max.getFloat();
		ubo_unpack.update(cmdList, 1, &uboData);

		quad->activate_position_uv(cmdList);
		quad->activateIndexBuffer(cmdList);
		quad->drawPrimitive(cmdList);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

}
