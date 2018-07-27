#include "deferredpass_unpack.h"
#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/render/god_ray.h"
#include "pathos/render/depth_of_field.h"

#include <algorithm>
using std::min;

#if defined(_DEBUG)
#include <iostream>
#endif

#define DOF 1

namespace pathos {

	static ConsoleVariable<float> cvar_tonemapping_exposure("r.tonemapping.exposure", 3.0, "exposure parameter of tone mapping pass");

	GLuint MeshDeferredRenderPass_Unpack::debug_godRayTexture() { return godRay->getTexture(); }

	MeshDeferredRenderPass_Unpack::MeshDeferredRenderPass_Unpack(
		GLuint gbuffer_tex0, GLuint gbuffer_tex1, GLuint gbuffer_tex2,
		unsigned int width, unsigned int height)
		: gbuffer_tex0(gbuffer_tex0), gbuffer_tex1(gbuffer_tex1), gbuffer_tex2(gbuffer_tex2)
	{
		// fullscreen quad
		quad = new PlaneGeometry(2.0f, 2.0f);
		createProgram();
		createResource_HDR(width, height);
		// post processing
		godRay = new GodRay(width, height);
		dof = new DepthOfField(width, height);
	}

	MeshDeferredRenderPass_Unpack::~MeshDeferredRenderPass_Unpack() {
		glDeleteProgram(program_ldr);
		glDeleteProgram(program_hdr);
		glDeleteProgram(program_tone_mapping);
		glDeleteFramebuffers(1, &fbo_hdr);
		glDeleteTextures(NUM_HDR_ATTACHMENTS, fbo_hdr_attachment);
		glDeleteProgram(program_blur);
		glDeleteFramebuffers(1, &fbo_blur);
		glDeleteTextures(1, &fbo_blur_attachment);
#if DOF
		glDeleteFramebuffers(1, &fbo_tone);
		glDeleteTextures(1, &fbo_tone_attachment);
#endif
		quad->dispose();
		delete quad;
		delete godRay;
		delete dof;
	}

	void MeshDeferredRenderPass_Unpack::createProgram() {
		createProgram_LDR();
		createProgram_HDR();
	}

	void MeshDeferredRenderPass_Unpack::createProgram_LDR() {
		string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
void main() {
	gl_Position = vec4(position, 1.0);
}
)";
		Shader vs(GL_VERTEX_SHADER);
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER);
		fs.loadSource("deferred_unpack_ldr.glsl");

		std::vector<Shader*> shaders = { &vs, &fs };
		program_ldr = pathos::createProgram(shaders);
	}

	void MeshDeferredRenderPass_Unpack::createProgram_HDR() {
		std::string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
void main() {
	gl_Position = vec4(position, 1.0);
}
)";
		
		Shader vs(GL_VERTEX_SHADER);
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER);
		fs.loadSource("deferred_unpack_hdr.glsl");

		// unpack hdr
		std::vector<Shader*> shaders = { &vs, &fs };
		program_hdr = pathos::createProgram(shaders);

		// tone mapping
		fs.loadSource("tone_mapping.glsl");
		program_tone_mapping = pathos::createProgram(shaders);
		uniform_tone_mapping_exposure = glGetUniformLocation(program_tone_mapping, "exposure");
		assert(uniform_tone_mapping_exposure != -1);

		// blur pass
		fs.loadSource("blur_pass.glsl");
		program_blur = pathos::createProgram(shaders);

		uniform_blur_horizontal = glGetUniformLocation(program_blur, "horizontal");
		assert(uniform_blur_horizontal != -1);
	}

	void MeshDeferredRenderPass_Unpack::createResource_HDR(unsigned int width, unsigned int height) {
		auto checkCurrentFramebufferStatus = []() -> void {
			// check if our framebuffer is ok
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#if defined(_DEBUG)
				std::cerr << "Cannot create a framebuffer for HDR" << std::endl;
#endif
				assert(0);
			}
		};

		// hdr resource
		glGenFramebuffers(1, &fbo_hdr);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_hdr);
		glGenTextures(NUM_HDR_ATTACHMENTS, fbo_hdr_attachment);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[0]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[1]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_hdr_attachment[0], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_hdr_attachment[1], 0);
		checkCurrentFramebufferStatus();

		// blur resource
		glGenFramebuffers(1, &fbo_blur);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur);
		glGenTextures(1, &fbo_blur_attachment);
		glBindTexture(GL_TEXTURE_2D, fbo_blur_attachment);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_blur_attachment, 0);
		checkCurrentFramebufferStatus();

		// tone mapping resource
#if DOF
		glGenFramebuffers(1, &fbo_tone);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_tone);
		glGenTextures(1, &fbo_tone_attachment);
		glBindTexture(GL_TEXTURE_2D, fbo_tone_attachment);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_tone_attachment, 0);
		checkCurrentFramebufferStatus();
#endif

		// restore default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void MeshDeferredRenderPass_Unpack::bindFramebuffer(bool hdr) {
		static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		if (hdr) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_hdr);
			glClearBufferfv(GL_COLOR, 0, zero);
			glClearBufferfv(GL_COLOR, 1, zero);
			glClearBufferfv(GL_COLOR, 2, zero);
		} else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glClearBufferfv(GL_COLOR, 0, zero);
		}
		use_hdr = hdr;
	}

	// This should be called after bindFramebuffer
	void MeshDeferredRenderPass_Unpack::setDrawBuffers(bool both) {
		if (!use_hdr) return;
		if (both) {
			GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(NUM_HDR_ATTACHMENTS, draw_buffers);
		} else {
			GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, draw_buffers);
		}
	}

	void MeshDeferredRenderPass_Unpack::setSunDepthMap(GLuint sunDepthMapTexture) {
		sunDepthMap = sunDepthMapTexture;
	}

	void MeshDeferredRenderPass_Unpack::render(Scene* scene, Camera* camera) {
		glUseProgram(program_ldr);

		// texture binding
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex2);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, sunDepthMap);

		glDisable(GL_DEPTH_TEST);

		quad->activate_position();
		quad->activateIndexBuffer();
		quad->draw();
		quad->deactivate();
		quad->deactivateIndexBuffer();

		glEnable(GL_DEPTH_TEST);
	}

	void MeshDeferredRenderPass_Unpack::renderHDR(Scene* scene, Camera* camera) {
		// prepare god ray image
		godRay->render(scene, camera);
		
		// bind
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_hdr);
		glUseProgram(program_hdr);

		// texture binding
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex2);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, sunDepthMap);

		glDisable(GL_DEPTH_TEST);

		// render HDR image
		quad->activate_position();
		quad->activateIndexBuffer();
		quad->draw();
		//quad->deactivate();
		//quad->deactivateIndexBuffer();

		// blur bright area
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_blur_attachment, 0);
		glUseProgram(program_blur);
		glUniform1i(uniform_blur_horizontal, GL_TRUE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[1]);
		quad->draw();
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_hdr_attachment[1], 0);
		glUniform1i(uniform_blur_horizontal, GL_FALSE);
		glBindTexture(GL_TEXTURE_2D, fbo_blur_attachment);
		quad->draw();

		// tone mapping
#if DOF
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_tone);
		GLenum tone_buffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, tone_buffers);
#else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
		glUseProgram(program_tone_mapping);
		glUniform1f(uniform_tone_mapping_exposure, cvar_tonemapping_exposure.getValue());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fbo_hdr_attachment[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, godRay->getTexture());

		//quad->activate_position();
		//quad->activateIndexBuffer();
		quad->draw();
		quad->deactivate();
		quad->deactivateIndexBuffer();

#if DOF
		dof->render(fbo_tone_attachment);
#endif

		// unbind
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_DEPTH_TEST);
	}

}