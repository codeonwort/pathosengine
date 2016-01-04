#include <pathos/mesh/shadow.h>
#include <pathos/mesh/shader.h>
#include <pathos/engine.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace pathos {

	vector<ShadowMap*> ShadowMap::instances;
	void ShadowMap::clearShadowTextures() {
		for (auto it = instances.begin(); it != instances.end(); it++) {
			(*it)->clearTexture();
		}
	}

	ShadowMap::ShadowMap(DirectionalLight* light, Camera* camera) :light(light), camera(camera) {
		instances.push_back(this);

		width = height = 1024;

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &shadowTexture);
		glBindTexture(GL_TEXTURE_2D, shadowTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexture, 0);

		static const GLenum buffs[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffs);

		glGenTextures(1, &debugTexture);
		glBindTexture(GL_TEXTURE_2D, debugTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, debugTexture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Cannot create a framebuffer for shadow map" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // return to default framebuffer

		string vshader = R"(#version 330 core
layout (location=0) in vec3 position;
uniform mat4 depthMVP;
void main() {
	gl_Position = depthMVP * vec4(position, 1);
}
)";
		string fshader = R"(#version 330 core
out vec4 color;
void main() {
	//color = vec4(gl_FragCoord.z);
	color = vec4(gl_FragCoord.z, 0, 0, 1);
}
)";
		program = createProgram(vshader, fshader);
	}

	ShadowMap::~ShadowMap() {
		instances.erase(std::find(instances.begin(), instances.end(), this));
		glDeleteTextures(1, &shadowTexture);
		glDeleteTextures(1, &debugTexture);
		glDeleteFramebuffers(1, &fbo);
		glDeleteProgram(program);
	}

	void ShadowMap::setGeometry(MeshGeometry* geom) { geometry = geom; }

	void ShadowMap::clearTexture() {
		static const GLfloat zero[] = { 0.0f };
		static const GLfloat one[] = { 1.0f };
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearBufferfv(GL_COLOR, 0, zero);
		glClearBufferfv(GL_DEPTH, 0, one);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowMap::activate(const glm::mat4 & modelMatrix) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, width, height);
		geometry->activateVertexBuffer(0);
		geometry->activateIndexBuffer();

		glUseProgram(program);
		GLfloat* lightDir = light->getDirection();
		glm::vec3 lightPos = glm::vec3(-lightDir[0], -lightDir[1], -lightDir[2]);
		lightPos = lightPos * 5.0f;
		glm::mat4 view = glm::lookAt(lightPos, glm::vec3(0, 0, -lightPos.z), glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::ortho(-20.0, 20.0, -20.0, 20.0, -10.0, 10.0);
		//projection = glm::perspective(glm::radians(110.0), 800.0 / 600, 1.0, 30.0);
		//glm::mat4 projection = calculateAABB(view);
		depthMVP = projection * view * modelMatrix;
		
		glUniformMatrix4fv(glGetUniformLocation(program, "depthMVP"), 1, GL_FALSE, &depthMVP[0][0]);
	}

	void ShadowMap::renderDepth() {
		glDrawElements(GL_TRIANGLES, geometry->getIndexCount(), GL_UNSIGNED_INT, (void*)0);
	}

	void ShadowMap::deactivate() {
		geometry->deactivateVertexBuffer(0);
		geometry->deactivateIndexBuffer();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		auto config = Engine::getConfig();
		glViewport(0, 0, config.width, config.height);
		glUseProgram(0);
	}

	glm::mat4 ShadowMap::calculateAABB(glm::mat4& lightView) {
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
		return glm::mat4(1.0f);
	}

}