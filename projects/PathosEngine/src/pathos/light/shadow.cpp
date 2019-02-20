#include "shadow.h"

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "pathos/shader/shader.h"
#include "pathos/engine.h"

#if defined(_DEBUG)
#include <iostream>
#endif

namespace pathos {

	ShadowMap::ShadowMap(unsigned int maxDirectionalLights) :maxLights(maxDirectionalLights) {
		width = height = 1024;

		// create framebuffer object
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glDrawBuffers(0, nullptr);

		// setup depth textures
		depthTextures.resize(maxLights);
		glGenTextures(maxLights, &depthTextures[0]);
		for (unsigned int i = 0; i < maxLights; ++i) {
			glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		// check if our framebuffer is ok
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextures[0], 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#if defined(_DEBUG)
			std::cerr << "Cannot create a framebuffer for shadow map" << std::endl;
#endif
			assert(0);
		}

		// return to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// create light depth program
		string vshader = R"(#version 430 core
layout (location=0) in vec3 position;
uniform mat4 depthMVP;
void main() {
	gl_Position = depthMVP * vec4(position, 1.0f);
}
)";
		string fshader = R"(#version 430 core
out vec4 color;
void main() {
	color = vec4(gl_FragCoord.z, 0.0f, 0.0f, 1.0f);
}
)";
		program = pathos::createProgram(vshader, fshader);
		uniform_depthMVP = glGetUniformLocation(program, "depthMVP");
		assert(uniform_depthMVP != -1);

		// @TODO: random values... how to adjust this automatically?
		projection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -100.0f, 100.0f);
	}

	ShadowMap::~ShadowMap() {
		glDeleteTextures(maxLights, &depthTextures[0]);
		glDeleteFramebuffers(1, &fbo);
		glDeleteProgram(program);
	}

	void ShadowMap::clearLightDepths(uint32_t numLights) {
		//static const GLfloat zero[] = { 0.0f };
		static const GLfloat one[] = { 1.0f };

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (auto i = 0u; i < numLights; ++i) {
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextures[i], 0);
			glClearBufferfv(GL_DEPTH, 0, one);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowMap::renderLightDepth(uint32_t lightIndex, DirectionalLight* light, MeshGeometry* modelGeometry, const glm::mat4& modelMatrix) {
		if (lightIndex >= maxLights) assert(0);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextures[lightIndex], 0);
		glViewport(0, 0, width, height);

		modelGeometry->activate_position();
		modelGeometry->activateIndexBuffer();

		// calculate uniform value
		GLfloat* lightDir = light->getDirection();
		glm::vec3 lightPos = glm::vec3(-lightDir[0], -lightDir[1], -lightDir[2]);
		view = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		//glm::mat4 projection = calculateAABB(view);
		glm::mat4 depthMVP = projection * view * modelMatrix;

		glUseProgram(program);
		glUniformMatrix4fv(uniform_depthMVP, 1, GL_FALSE, &(depthMVP[0][0]));
		
		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		modelGeometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		modelGeometry->deactivate();
		modelGeometry->deactivateIndexBuffer();
		glUseProgram(0);

		// restore original viewport
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// TODO: remove the global access
		auto config = gEngine->getConfig();
		glViewport(0, 0, config.windowWidth, config.windowHeight);
	}

	void ShadowMap::activate(GLuint materialProgram, const vector<DirectionalLight*>& lights, unsigned int textureBinding, const glm::mat4& modelMatrix) {
		const auto numLights = lights.size();
		//textureBindings.resize(numLights);
		textureBindings.resize(maxLights);
		for (auto i = 0u; i < numLights; ++i) textureBindings[i] = textureBinding + i;

		static glm::mat4 bias(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0);
		static vector<glm::mat4> depthMVPbiased;
		glm::mat4 depthMVP = projection * view * modelMatrix;
		depthMVPbiased.resize(numLights);
		for (auto lightIndex = 0u; lightIndex < numLights; ++lightIndex) {
			depthMVPbiased[lightIndex] = bias * depthMVP;
			glActiveTexture(GL_TEXTURE0 + textureBindings[lightIndex]);
			glBindTexture(GL_TEXTURE_2D, depthTextures[lightIndex]);
		}
		// not necessary, but to eliminate a warning about invalid texture binding...
		for (auto lightIndex = numLights; lightIndex < maxLights; ++lightIndex) {
			glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(textureBinding + lightIndex));
			textureBindings[lightIndex] = textureBindings[0];
			glBindTexture(GL_TEXTURE_2D, depthTextures[0]);
		}

		/*
		GLint u_depthMVP = glGetUniformLocation(materialProgram, "depthMVP");
		glUniformMatrix4fv(u_depthMVP, numLights, false, &(depthMVPbiased[0][0][0]));*/
		GLint u_depthSampler = glGetUniformLocation(materialProgram, "depthSampler");
		//glUniform1iv(u_depthSampler, numLights, &textureBindings[0]);
		glUniform1iv(u_depthSampler, maxLights, &textureBindings[0]);
		for (auto i = 0u; i < numLights; ++i) {
			std::string u_depthMVP = "depthMVP[" + to_string(i) + "]";
			glUniformMatrix4fv(glGetUniformLocation(materialProgram, u_depthMVP.c_str()), 1, false, &(depthMVPbiased[i][0][0]));
		}
	}

	void ShadowMap::deactivate(GLuint materialProgram, unsigned int textureBinding) {
		static_cast<void>(materialProgram);
		for (auto i = 0u; i < textureBindings.size(); ++i) {
			glActiveTexture(GL_TEXTURE0 + textureBinding + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void ShadowMap::addShaderCode(VertexShaderSource& vs, FragmentShaderSource& fs, unsigned int maxLights) {
		vs.setUseNormal(true);
		vs.uniform("uint", "numDirLights");
		vs.uniform("mat4", "depthMVP", maxLights);
		vs.outVar("vec4", "shadowMapCoord", maxLights);

		vs.mainCode("for (unsigned int i = 0; i < numDirLights; ++i) {");
		vs.mainCode("  vs_out.shadowMapCoord[i] = depthMVP[i] * vec4(position, 1);");
		vs.mainCode("}");

		fs.inVar("vec3", "normal");
		fs.inVar("vec4", "shadowMapCoord", maxLights);
		fs.uniform("sampler2DShadow", "depthSampler", maxLights);

		fs.mainCode("vec3 sm_normal = normalize(fs_in.normal);");
		fs.mainCode("for(unsigned int i=0; i<numDirLights; ++i) {");
		fs.mainCode("  float cosTheta = clamp(dot(sm_normal, -dirLightDirs[i]), 0.0f, 1.0f);");
		fs.mainCode("  float bias = clamp(0.005 * tan(acos(cosTheta)), 0.0f, 0.1f);");
		// w component is actually always 1.0 if orthogonal projection was used
		// shadow2D sampling only results in 0.0 or 1.0
		fs.mainCode("  float w = fs_in.shadowMapCoord[i].w;");
		fs.mainCode("  float depth_test = texture(depthSampler[i], vec3(fs_in.shadowMapCoord[i].xy / w, (fs_in.shadowMapCoord[i].z - bias) / w));");
		fs.mainCode("  if(depth_test < .5) visibility *= 0.2f;");
		fs.mainCode("}");
	}

	//glm::mat4 ShadowMap::calculateAABB(glm::mat4& lightView) {
		/*double fov = camera->getFieldOfView();
		double ratio = camera->getAspectRatio();
		double zNear = camera->getNearClipPlaneDistance();
		double zFar = camera->getFarClipPlaneDistance() / 5;

		double hw = zNear * tan(glm::radians(fov) / 2.0); // half of width at near plane
		double hh = hw / ratio; // half of height at near plane
		double HW = hw * zFar / zNear; // at far plane
		double HH = HW / ratio; // at far plane
		vertices[0] = glm::vec4(-hw, -hh, zNear, 1);
		vertices[1] = glm::vec4(-hw, hh, zNear, 1);
		vertices[2] = glm::vec4(hw, -hh, zNear, 1);
		vertices[3] = glm::vec4(hw, hh, zNear, 1);
		vertices[4] = glm::vec4(-HW, -HH, zFar, 1);
		vertices[5] = glm::vec4(-HW, HH, zFar, 1);
		vertices[6] = glm::vec4(HW, -HH, zFar, 1);
		vertices[7] = glm::vec4(HW, HH, zFar, 1);

		// transform edges of the camera frustum from camera space to light space
		// and find AABB in light space that entirely covers camera frustum
		glm::mat4 cameraSpaceInv = camera->getWorldToCameraMatrix();
		cameraSpaceInv = glm::inverse(cameraSpaceInv);
		double left = 100000, right = -1000000, bottom = 100000, top = -1000000, near = 100000, far = 1000000;
		for (int i = 0; i < 8; i++) {
			vertices[i] = lightView * cameraSpaceInv * vertices[i];
			//cout << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;
			if (vertices[i].x < left) left = vertices[i].x;
			if (vertices[i].x > right) right = vertices[i].x;
			if (vertices[i].y < bottom) bottom = vertices[i].y;
			if (vertices[i].y > top) top = vertices[i].y;
			if (vertices[i].z < near) near = vertices[i].z;
			if (vertices[i].z > far) far = vertices[i].z;
		}
		//cout << left << ", " << bottom << " : " << right << ", " << top << " : " << near << ", " << far << endl;
		return glm::ortho(left, right, bottom, top, near, far);*/
		//return glm::mat4(1.0f);
	//}

}
