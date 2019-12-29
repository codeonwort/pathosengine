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
		: use_hdr(true)
		, fbo_hdr(0xffffffff)
	{
	}

	MeshDeferredRenderPass_Unpack::~MeshDeferredRenderPass_Unpack() {
		CHECK(destroyed);
	}

	void MeshDeferredRenderPass_Unpack::initializeResources(RenderCommandList& cmdList) {
		// fullscreen quad
		quad = new PlaneGeometry(2.0f, 2.0f);
		createProgram_LDR();
		createProgram_HDR();
		createResource_HDR(cmdList);
	}

	void MeshDeferredRenderPass_Unpack::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			cmdList.deleteProgram(program_ldr);
			cmdList.deleteProgram(program_hdr);
			cmdList.deleteFramebuffers(1, &fbo_hdr);
			quad->dispose();
			delete quad;
		}
		destroyed = true;
	}

	void MeshDeferredRenderPass_Unpack::createProgram_LDR() {
		string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
void main() {
	gl_Position = vec4(position, 1.0);
}
)";
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Unpack_LDR");
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Unpack_LDR");
		fs.loadSource("deferred_unpack_ldr.glsl");

		program_ldr = pathos::createProgram(vs, fs, "UnpackLDR");
	}

	void MeshDeferredRenderPass_Unpack::createProgram_HDR() {
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
		
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Unpack_HDR");
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Unpack_HDR");
		fs.loadSource("deferred_unpack_hdr.glsl");

		program_hdr = pathos::createProgram(vs, fs, "UnpackHDR");
		ubo_unpack.init<UBO_Unpack>();
	}

	void MeshDeferredRenderPass_Unpack::createResource_HDR(RenderCommandList& cmdList) {
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
		cmdList.createFramebuffers(1, &fbo_hdr);
		cmdList.namedFramebufferTexture(fbo_hdr, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo_hdr, GL_COLOR_ATTACHMENT1, sceneContext.sceneBloom, 0);
		cmdList.namedFramebufferDrawBuffers(fbo_hdr, 2, hdr_draw_buffers);
		checkFramebufferStatus(fbo_hdr);
	}

	void MeshDeferredRenderPass_Unpack::bindFramebuffer(RenderCommandList& cmdList, bool hdr) {
		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		if (hdr) {
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_hdr);
			cmdList.clearBufferfv(GL_COLOR, 0, zero);
			cmdList.clearBufferfv(GL_COLOR, 1, zero);
			cmdList.clearBufferfv(GL_COLOR, 2, zero);
		} else {
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			//cmdList.clearBufferfv(GL_COLOR, 0, zero);
		}
		use_hdr = hdr;
	}

	void MeshDeferredRenderPass_Unpack::renderLDR(RenderCommandList& cmdList, Scene* scene, Camera* camera) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.useProgram(program_ldr);

		GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(6, sceneContext.cascadedShadowMap);

		cmdList.disable(GL_DEPTH_TEST);

		quad->activate_position(cmdList);
		quad->activateIndexBuffer(cmdList);
		quad->drawPrimitive(cmdList);
		quad->deactivate(cmdList);
		quad->deactivateIndexBuffer(cmdList);

		cmdList.enable(GL_DEPTH_TEST);
	}

	void MeshDeferredRenderPass_Unpack::renderHDR(RenderCommandList& cmdList, Scene* scene, Camera* camera) {
		SCOPED_DRAW_EVENT(UnpackHDR);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.useProgram(program_hdr);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_hdr);

		GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
		cmdList.bindTextures(0, 3, gbuffer_textures);
		// #todo-binding: Why did I bind csm at 6??? Let's bind ssao at 5 for now...
		cmdList.bindTextureUnit(5, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(6, sceneContext.cascadedShadowMap);

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

		// render HDR image
		quad->activate_position_uv(cmdList);
		quad->activateIndexBuffer(cmdList);
		quad->drawPrimitive(cmdList);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

}
