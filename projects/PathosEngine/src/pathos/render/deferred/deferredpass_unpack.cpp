#include "deferredpass_unpack.h"
#include "pathos/engine.h"
#include "pathos/render/god_ray.h"
#include "pathos/render/depth_of_field.h"

#include <algorithm>
using std::min;

#if defined(_DEBUG)
#include <iostream>
#endif

#define DOF 0

namespace pathos {

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
		glDeleteProgram(program);
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
		program = pathos::createProgram(shaders);

#define GET_UNIFORM(z) { uniform_ldr_##z = glGetUniformLocation(program, #z); assert(uniform_ldr_##z != -1); }
		GET_UNIFORM(eyeDirection);
		GET_UNIFORM(numDirLights);
		GET_UNIFORM(dirLightDirs);
		GET_UNIFORM(dirLightColors);
		GET_UNIFORM(numPointLights);
		GET_UNIFORM(pointLightPos);
		GET_UNIFORM(pointLightColors);
#undef GET_UNIFORM
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

		std::vector<Shader*> shaders = { &vs, &fs };
		program_hdr = pathos::createProgram(shaders);

		// tone mapping
		fs.loadSource("tone_mapping.glsl");
		program_tone_mapping = pathos::createProgram(shaders);

		// blur pass
		fs.loadSource("blur_pass.glsl");
		program_blur = pathos::createProgram(shaders);

#define GET_UNIFORM(z) { uniform_hdr_##z = glGetUniformLocation(program_hdr, #z); assert(uniform_hdr_##z != -1); }
		GET_UNIFORM(eyeDirection);
		GET_UNIFORM(eyePosition);
		GET_UNIFORM(numDirLights);
		GET_UNIFORM(dirLightDirs);
		GET_UNIFORM(dirLightColors);
		GET_UNIFORM(numPointLights);
		GET_UNIFORM(pointLightPos);
		GET_UNIFORM(pointLightColors);
#undef GET_UNIFORM
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
		if (hdr) {
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_hdr);
			static const GLfloat zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			glClearBufferfv(GL_COLOR, 0, zero);
			glClearBufferfv(GL_COLOR, 1, zero);
			glClearBufferfv(GL_COLOR, 2, zero);
		} else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	// This should be called after bindFramebuffer
	void MeshDeferredRenderPass_Unpack::setDrawBuffers(bool both) {
		if (both) {
			GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(NUM_HDR_ATTACHMENTS, draw_buffers);
		} else {
			GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, draw_buffers);
		}
	}

	void MeshDeferredRenderPass_Unpack::render(Scene* scene, Camera* camera) {
		glUseProgram(program);

		// texture binding
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gbuffer_tex2);

		// uniform: light
		uploadDirectionalLightUniform(scene, MAX_DIRECTIONAL_LIGHTS, false);
		uploadPointLightUniform(scene, MAX_POINT_LIGHTS, false);

		// uniform: camera
		const glm::vec3& eye = glm::vec3(camera->getViewMatrix() * glm::vec4(camera->getEyeVector(), 0.0f));
		glUniform3fv(uniform_ldr_eyeDirection, 1, &eye[0]);

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

		// uniform: light
		uploadDirectionalLightUniform(scene, MAX_DIRECTIONAL_LIGHTS, true);
		uploadPointLightUniform(scene, MAX_POINT_LIGHTS, true);

		// uniform: camera
		const glm::vec3& eyeDir = glm::vec3(camera->getViewMatrix() * glm::vec4(camera->getEyeVector(), 0.0f));
		glUniform3fv(uniform_hdr_eyeDirection, 1, &eyeDir[0]);

		const glm::vec3& eyePos = glm::vec3(camera->getViewMatrix() * glm::vec4(camera->getPosition(), 1.0f));
		glUniform3fv(uniform_hdr_eyePosition, 1, &eyePos[0]);

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

	void MeshDeferredRenderPass_Unpack::uploadDirectionalLightUniform(Scene* scene, unsigned int maxDirectionalLights, bool hdr) {
		if ((hdr && program_hdr == 0) || (!hdr && program == 0)) return;
		unsigned int numDirLights = std::min(static_cast<unsigned int>(scene->directionalLights.size()), maxDirectionalLights);
		if (hdr) {
			glUniform1ui(uniform_hdr_numDirLights, numDirLights);
			if (numDirLights > 0) {
				glUniform3fv(uniform_hdr_dirLightDirs, numDirLights, scene->getDirectionalLightDirectionBuffer());
				glUniform3fv(uniform_hdr_dirLightColors, numDirLights, scene->getDirectionalLightColorBuffer());
			}
		} else {
			glUniform1ui(uniform_ldr_numDirLights, numDirLights);
			if (numDirLights > 0) {
				glUniform3fv(uniform_ldr_dirLightDirs, numDirLights, scene->getDirectionalLightDirectionBuffer());
				glUniform3fv(uniform_ldr_dirLightColors, numDirLights, scene->getDirectionalLightColorBuffer());
			}
		}
	}
	void MeshDeferredRenderPass_Unpack::uploadPointLightUniform(Scene* scene, unsigned int maxPointLights, bool hdr) {
		if ((hdr && program_hdr == 0) || (!hdr && program == 0)) return;
		unsigned int numPointLights = std::min(static_cast<unsigned int>(scene->pointLights.size()), maxPointLights);
		if (hdr) {
			glUniform1ui(uniform_hdr_numPointLights, numPointLights);
			if (numPointLights) {
				glUniform3fv(uniform_hdr_pointLightPos, numPointLights, scene->getPointLightPositionBuffer());
				glUniform3fv(uniform_hdr_pointLightColors, numPointLights, scene->getPointLightColorBuffer());
			}
		} else {
			glUniform1ui(uniform_ldr_numPointLights, numPointLights);
			if (numPointLights) {
				glUniform3fv(uniform_ldr_pointLightPos, numPointLights, scene->getPointLightPositionBuffer());
				glUniform3fv(uniform_ldr_pointLightColors, numPointLights, scene->getPointLightColorBuffer());
			}
		}
	}

}