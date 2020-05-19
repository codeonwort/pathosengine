#include "shadow_omni.h"
#include "pathos/engine.h"
#include "pathos/util/log.h"
#include "pathos/light/point_light_component.h"

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

namespace pathos {

	OmnidirectionalShadow::OmnidirectionalShadow(unsigned int maxPointLights): maxLights(maxPointLights) {
		width = height = 1024;
		// @TODO: random values
		lightNearZ = 1.0f;
		lightFarZ = 200.0f;

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glDrawBuffers(0, nullptr);

		depthTextures.resize(maxLights);
		glGenTextures(maxLights, &depthTextures[0]);
		for (unsigned int i = 0; i < maxLights; ++i) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthTextures[i]);
			glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_DEPTH_COMPONENT32F, width, height);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

		// check if our framebuffer is ok
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextures[0], 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LOG(LogFatal, "Cannot create a framebuffer for omnidirectional shadow map");
			assert(0);
		}

		// return to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// create light depth program
		string vshader = R"(#version 430 core
layout (location=0) in vec3 position;
uniform mat4 depthMVP;
void main() {
	gl_Position = depthMVP * vec4(position, 1);
}
)";
		string fshader = R"(#version 430 core
out vec4 color;
void main() {
	color = vec4(gl_FragCoord.z, 0, 0, 1);
}
)";
		program = pathos::createProgram(vshader, fshader, "OmnidirectionalShadowMapping");
		uniform_depthMVP = glGetUniformLocation(program, "depthMVP");
		assert(uniform_depthMVP != -1);
	}

	OmnidirectionalShadow::~OmnidirectionalShadow() {
		glDeleteTextures(maxLights, &depthTextures[0]);
		glDeleteFramebuffers(1, &fbo);
		glDeleteProgram(program);
	}

	void OmnidirectionalShadow::clearLightDepths(unsigned int numLights) {
		static const GLfloat one[] = { 1.0f };

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (auto k = 0u; k < numLights; ++k) {
			for (int i = 0; i < 6; i++) {
				auto face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, depthTextures[k], 0);
				glClearBufferfv(GL_DEPTH, 0, one);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OmnidirectionalShadow::renderLightDepth(RenderCommandList& cmdList, uint32 lightIndex, PointLightProxy* light, MeshGeometry* mesh, const glm::mat4& modelMatrix) {
		if (lightIndex >= maxLights) {
			assert(0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (int i = 0; i < 6; i++) {
			auto face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, depthTextures[lightIndex], 0);
		}
		glViewport(0, 0, width, height);

		mesh->activate_position(cmdList);
		mesh->activateIndexBuffer(cmdList);

		glUseProgram(program);

		glm::vec3 lightPos = light->position;
		glm::vec3 directions[6] = { glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1) };
		glm::vec3 ups[6] = { glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0) };
		glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)width / height, lightNearZ, lightFarZ);

		for (int i = 0; i < 6; i++) {
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, depthTextures[lightIndex], 0);
			glm::mat4 view = glm::lookAt(lightPos, lightPos + directions[i], ups[i]);
			glm::mat4 depthMVP = projection * view * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(program, "depthMVP"), 1, GL_FALSE, &(depthMVP[0][0]));
			// draw call
			mesh->drawPrimitive(cmdList);
		}

		// restore original viewport
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// TODO: remove the global access
		auto config = gEngine->getConfig();
		glViewport(0, 0, config.windowWidth, config.windowHeight);
	}

	void OmnidirectionalShadow::activate(GLuint materialProgram, const vector<PointLightProxy*>& lights, unsigned int textureBinding, const glm::mat4& modelMatrix)
	{
		static_cast<void>(modelMatrix);

		const auto numLights = lights.size();
		//textureBindings.resize(numLights);
		textureBindings.resize(maxLights);

		//for (auto i = 0; i < numLights; ++i) {
		for (auto i = 0u; i < maxLights; ++i) {
			if (i < numLights) {
				textureBindings[i] = textureBinding + i;
				glActiveTexture(GL_TEXTURE0 + textureBindings[i]);
				glBindTexture(GL_TEXTURE_CUBE_MAP, depthTextures[i]);
			} else {
				textureBindings[i] = textureBindings[0];
				glActiveTexture(GL_TEXTURE0 + textureBindings[i]);
				glBindTexture(GL_TEXTURE_CUBE_MAP, depthTextures[0]);
			}
		}
		
		//glUniform1iv(glGetUniformLocation(materialProgram, "omniShadow_depthSampler"), numLights, &textureBindings[0]);
		glUniform1iv(glGetUniformLocation(materialProgram, "omniShadow_depthSampler"), maxLights, &textureBindings[0]);
		glUniform1f(glGetUniformLocation(materialProgram, "f_plus_n"), lightNearZ + lightFarZ);
		glUniform1f(glGetUniformLocation(materialProgram, "f_minus_n"), lightFarZ - lightNearZ);
		glUniform1f(glGetUniformLocation(materialProgram, "f_mult_n"), lightFarZ * lightNearZ);
	}
	void OmnidirectionalShadow::deactivate(GLuint materialProgram, unsigned int textureBinding) {
		static_cast<void>(materialProgram);
		for (auto i = 0u; i < textureBindings.size(); ++i) {
			glActiveTexture(GL_TEXTURE0 + textureBinding + i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
	}

	void OmnidirectionalShadow::addShaderCode(VertexShaderSource& vs, FragmentShaderSource& fs, unsigned int maxLights) {
		vs.setUseNormal(true);
		vs.outVar("vec3", "omniShadowCoord");
		vs.mainCode("vs_out.omniShadowCoord = vec3(modelTransform * vec4(position, 1.0f));");

		fs.inVar("vec3", "normal");
		fs.inVar("vec3", "omniShadowCoord");
		fs.uniform("uint", "numPointLights");
		fs.uniform("samplerCubeShadow", "omniShadow_depthSampler", maxLights);
		fs.uniform("float", "f_plus_n");
		fs.uniform("float", "f_minus_n");
		fs.uniform("float", "f_mult_n");

		fs.mainCode("unsigned int omni_count = 0;");
		fs.mainCode("for (unsigned int i = 0; i < numPointLights; ++i) {");
		fs.mainCode("  vec3 dir = fs_in.omniShadowCoord - pointLightPos[i];");
		fs.mainCode("  float cosTheta = clamp(dot(normalize(fs_in.normal), -normalize(dir)), 0.0f, 1.0f);");
		fs.mainCode("  float bias = clamp(0.005 * tan(acos(cosTheta)), 0.0f, 0.1f);");
		fs.mainCode("  float localZ = max(abs(dir.x), max(abs(dir.y), abs(dir.z)));");
		fs.mainCode("  localZ = f_plus_n/f_minus_n - (2*f_mult_n)/f_minus_n/localZ;");
		fs.mainCode("  localZ = (localZ - bias + 1) * 0.5;");
		fs.mainCode("  float depth_test = texture(omniShadow_depthSampler[i], vec4(normalize(dir), localZ));");
		fs.mainCode("  if(depth_test > .5) ++omni_count;");
		fs.mainCode("}");
		fs.mainCode("if (omni_count == 0) visibility *= 0.5f;");
	}

}
