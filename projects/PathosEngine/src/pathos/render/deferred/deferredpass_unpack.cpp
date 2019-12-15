#include "deferredpass_unpack.h"
#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/render/god_ray.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/postprocessing/depth_of_field.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include <algorithm>

using std::min;

#define DOF 0

namespace pathos {

	struct UBO_Unpack {
		glm::ivec4 enabledTechniques1; // (shadow, fog, ?, ?)
		glm::vec4 fogColor;
		glm::vec4 fogParams;           // (bottomY, topY, ?, ?)
		glm::vec4 bloomParams;
	};

	struct UBO_ToneMapping {
		float exposure;
		float gamma;
	};

	static ConsoleVariable<int32_t> cvar_enable_shadow("r.shadow", 1, "0 = disable shadow, 1 = enable shadow");
	
	static ConsoleVariable<int32_t> cvar_enable_fog("r.fog", 1, "0 = disable fog, 1 = enable fog");
	static ConsoleVariable<float> cvar_fog_bottom("r.fog.bottom", 0.0f, "bottom Y");
	static ConsoleVariable<float> cvar_fog_top("r.fog.top", 200.0f, "top Y");
	static ConsoleVariable<float> cvar_fog_attenuation("r.fog.attenuation", 0.01f, "fog attenuation coefficient");

	static ConsoleVariable<float> cvar_bloom_strength("r.bloom.strength", 4.0f, "Bloom strength");
	static ConsoleVariable<float> cvar_bloom_min("r.bloom.min", 0.8f, "Minimum bloom");
	static ConsoleVariable<float> cvar_bloom_max("r.bloom.max", 1.2f, "Maximum bloom");

	static ConsoleVariable<int32_t> cvar_enable_dof("r.dof", 1, "0 = disable DoF, 1 = enable DoF");

	static ConsoleVariable<float> cvar_tonemapping_exposure("r.tonemapping.exposure", 1.0f, "exposure parameter of tone mapping pass");
	static ConsoleVariable<float> cvar_gamma("r.gamma", 2.2f, "gamma correction");

	MeshDeferredRenderPass_Unpack::MeshDeferredRenderPass_Unpack() {
		// post processing
		dof = new DepthOfField;
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
		dof->initializeResources(cmdList);
	}

	void MeshDeferredRenderPass_Unpack::destroyResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			cmdList.deleteProgram(program_ldr);
			cmdList.deleteProgram(program_hdr);
			cmdList.deleteProgram(program_tone_mapping);
			cmdList.deleteProgram(program_blur);
			cmdList.deleteFramebuffers(1, &fbo_hdr);
			cmdList.deleteFramebuffers(1, &fbo_blur);
			cmdList.deleteFramebuffers(1, &fbo_tone);
			quad->dispose();
			dof->releaseResources(cmdList);
			delete quad;
			delete dof;
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

		std::vector<Shader*> shaders = { &vs, &fs };
		program_ldr = pathos::createProgram(shaders, "UnpackLDR");
	}

	void MeshDeferredRenderPass_Unpack::createProgram_HDR() {
		std::string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
void main() {
	gl_Position = vec4(position, 1.0);
}
)";
		
		Shader vs(GL_VERTEX_SHADER, "VS_Deferred_Unpack_HDR");
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER, "FS_Deferred_Unpack_HDR");
		fs.loadSource("deferred_unpack_hdr.glsl");

		// unpack hdr
		std::vector<Shader*> shaders = { &vs, &fs };
		program_hdr = pathos::createProgram(shaders, "UnpackHDR");
		ubo_unpack.init<UBO_Unpack>();

		// tone mapping
		fs.loadSource("tone_mapping.glsl");
		program_tone_mapping = pathos::createProgram(shaders, "ToneMapping");
		ubo_tone_mapping.init<UBO_ToneMapping>();

		// blur pass
		fs.loadSource("blur_pass.glsl");
		program_blur = pathos::createProgram(shaders, "BlurPass");
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

		// blur resource
		cmdList.createFramebuffers(1, &fbo_blur);
		// #todo-framebuffer: This is set twice each with different texture. Any performance issue? Maybe I need two FBOs?
		cmdList.namedFramebufferTexture(fbo_blur, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloomTemp, 0);
		cmdList.namedFramebufferDrawBuffer(fbo_blur, GL_COLOR_ATTACHMENT0);
		checkFramebufferStatus(fbo_blur);

		// tone mapping resource
		cmdList.createFramebuffers(1, &fbo_tone);
		cmdList.namedFramebufferTexture(fbo_tone, GL_COLOR_ATTACHMENT0, sceneContext.toneMappingResult, 0);
		cmdList.namedFramebufferDrawBuffer(fbo_tone, GL_COLOR_ATTACHMENT0);
		checkFramebufferStatus(fbo_tone);
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

	// #todo-post-processing: Extract post-processing passes out of this function
	void MeshDeferredRenderPass_Unpack::renderHDR(RenderCommandList& cmdList, Scene* scene, Camera* camera) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		{
			SCOPED_DRAW_EVENT(UnpackHDR);

			// bind
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_hdr);
			cmdList.useProgram(program_hdr);

			GLuint gbuffer_textures[] = { sceneContext.gbufferA, sceneContext.gbufferB, sceneContext.gbufferC };
			cmdList.bindTextures(0, 3, gbuffer_textures);
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
			quad->activate_position(cmdList);
			quad->activateIndexBuffer(cmdList);
			quad->drawPrimitive(cmdList);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		//cmdList.flushAllCommands(); // #todo-renderdoc: debugging

		{
			SCOPED_DRAW_EVENT(BloomPass);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_blur);
			cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloomTemp, 0);
			cmdList.useProgram(program_blur);
			cmdList.uniform1i(uniform_blur_horizontal, GL_TRUE);
			cmdList.bindTextureUnit(0, sceneContext.sceneBloom);
			quad->drawPrimitive(cmdList);
			cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

			cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sceneContext.sceneBloom, 0);
			cmdList.uniform1i(uniform_blur_horizontal, GL_FALSE);
			cmdList.bindTextureUnit(0, sceneContext.sceneBloomTemp);
			quad->drawPrimitive(cmdList);
			cmdList.framebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		//cmdList.flushAllCommands(); // #todo-renderdoc: debugging

		{
			SCOPED_DRAW_EVENT(ToneMapping);

			if (cvar_enable_dof.getInt() != 0) {
				cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_tone);
			} else {
				cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			}

			cmdList.useProgram(program_tone_mapping);
			UBO_ToneMapping uboData_toneMapping;
			uboData_toneMapping.exposure = cvar_tonemapping_exposure.getValue();
			uboData_toneMapping.gamma = cvar_gamma.getValue();
			ubo_tone_mapping.update(cmdList, 0, &uboData_toneMapping);

			GLuint tonemapping_attachments[] = { sceneContext.sceneColor, sceneContext.sceneBloom, sceneContext.godRayResult };
			cmdList.bindTextures(0, 3, tonemapping_attachments);

			quad->drawPrimitive(cmdList);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		//cmdList.flushAllCommands(); // #todo-renderdoc: debugging

		if (cvar_enable_dof.getInt() != 0) {
			constexpr GLuint dofRenderTarget = 0; // default framebuffer
			// #todo-post-processing
			//dof->setInput(EPostProcessInput::PPI_0, sceneContext.toneMappingResult);
			//dof->setOutput(EPostProcessOutput::PPO_0, dofRenderTarget);
			dof->renderPostProcess(cmdList, quad);
		}

		//cmdList.flushAllCommands(); // #todo-renderdoc: debugging
	}

}
