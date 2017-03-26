#include <pathos/mesh/shadow.h>
#include <pathos/render/shader.h>
#include <pathos/engine.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>

namespace pathos {

	vector<ShadowMethod*> ShadowMethod::instances;
	void ShadowMethod::clearShadowTextures() {
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
		program = createProgram(vshader, fshader);
	}

	ShadowMap::~ShadowMap() {
		instances.erase(std::find(instances.begin(), instances.end(), this));
		glDeleteTextures(1, &shadowTexture);
		glDeleteTextures(1, &debugTexture);
		glDeleteFramebuffers(1, &fbo);
		glDeleteProgram(program);
	}

	void ShadowMap::clearTexture() {
		static const GLfloat zero[] = { 0.0f };
		static const GLfloat one[] = { 1.0f };
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearBufferfv(GL_COLOR, 0, zero);
		glClearBufferfv(GL_DEPTH, 0, one);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ShadowMap::renderDepth() {
		// set program
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, width, height);
		modelGeometry->activateVertexBuffer(0);
		modelGeometry->activateIndexBuffer();

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
		
		// draw call
		modelGeometry->draw();

		// unset program
		modelGeometry->deactivateVertexBuffer(0);
		modelGeometry->deactivateIndexBuffer();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		auto config = Engine::getConfig();
		glViewport(0, 0, config.width, config.height);
		glUseProgram(0);
	}

	void ShadowMap::activate(GLuint materialProgram) {
		glm::mat4 bias(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);
		glm::mat4 depthMVPbiased = bias * depthMVP;
		GLuint depthMVPLoc = glGetUniformLocation(materialProgram, "depthMVP");
		glUniformMatrix4fv(depthMVPLoc, 1, false, &depthMVPbiased[0][0]);
		glUniform3fv(glGetUniformLocation(materialProgram, "shadowLight"), 1, light->getDirection());
		glUniform1i(glGetUniformLocation(materialProgram, "depthSampler"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowTexture);
	}

	void ShadowMap::deactivate() {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void ShadowMap::addShaderCode(VertexShaderSource& vs, FragmentShaderSource& fs) {
		vs.setUseNormal(true);
		vs.outVar("vec4", "shadowCoord");
		vs.uniformMat4("depthMVP");
		vs.mainCode("vs_out.shadowCoord = depthMVP * vec4(position, 1);");

		fs.inVar("vec3", "normal");
		fs.inVar("vec4", "shadowCoord");
		fs.textureSamplerShadow("depthSampler");
		fs.uniform("vec3", "shadowLight");
		fs.mainCode("float cosTheta = clamp(dot(normalize(fs_in.normal), -shadowLight), 0, 1);");
		fs.mainCode("float bias = clamp(0.05 * tan(acos(cosTheta)), 0, 0.1);");
		fs.mainCode("visibility = texture(depthSampler, vec3(fs_in.shadowCoord.xy, (fs_in.shadowCoord.z-bias)/fs_in.shadowCoord.w));");
		fs.mainCode("if(visibility < .5) visibility = .5;");
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

	////////////////////////////////////////////////////////////////////////
	// omnidirectional shadow
	OmnidirectionalShadow::OmnidirectionalShadow(PointLight* light, Camera* camera) :light(light), camera(camera) {
		// constructor
		instances.push_back(this);

		width = height = 1024;

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glGenTextures(1, &shadowTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowTexture);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexture, 0);

		glGenTextures(1, &debugTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, debugTexture);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		for (int i = 0; i < 6; i++) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, debugTexture, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "[ERROR] Cannot create a framebuffer for omnidirectional shadow map" << std::endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // return to default framebuffer

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
	color = vec4(pow(gl_FragCoord.z, 32), 0, 0, 1);
}
)";
		program = createProgram(vshader, fshader);
	}

	void OmnidirectionalShadow::clearTexture() {
		static const GLfloat zero[] = { 0.0f };
		static const GLfloat one[] = { 1.0f };
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		for (int i = 0; i < 6; i++){
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, shadowTexture, 0);
			glClearBufferfv(GL_DEPTH, 0, one);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, debugTexture, 0);
			glClearBufferfv(GL_COLOR, 0, zero);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OmnidirectionalShadow::renderDepth() {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, width, height);

		// set program
		glUseProgram(program);
		
		modelGeometry->activateVertexBuffer(0);
		modelGeometry->activateIndexBuffer();

		lightNearZ = 0.05f;
		lightFarZ = 20.0f;
		glm::vec3 lightPos = light->getPositionVector();
		glm::vec3 directions[6] = { glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1) };
		glm::vec3 ups[6] = { glm::vec3(0, -1, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0), glm::vec3(0, -1, 0) };
		glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)width / height, lightNearZ, lightFarZ);

		for (int i = 0; i < 6; i++){
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, shadowTexture, 0);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, debugTexture, 0);
			glm::mat4 view = glm::lookAt(lightPos, lightPos + directions[i], ups[i]);
			glm::mat4 depthMVP = projection * view * modelMatrix;
			glUniformMatrix4fv(glGetUniformLocation(program, "depthMVP"), 1, GL_FALSE, &(depthMVP[0][0]));
			// draw call
			modelGeometry->draw();
		}

		// unset program
		modelGeometry->deactivateVertexBuffer(0);
		modelGeometry->deactivateIndexBuffer();

		// return to default setting
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		auto config = Engine::getConfig();
		glViewport(0, 0, config.width, config.height);
		glUseProgram(0);
	}

	void OmnidirectionalShadow::activate(GLuint materialProgram) {
		glUniform3fv(glGetUniformLocation(materialProgram, "shadowLightPos"), 1, light->getPosition());
		glUniform1i(glGetUniformLocation(materialProgram, "depthSampler"), 1);
		glUniform1f(glGetUniformLocation(materialProgram, "f_plus_n"), lightNearZ + lightFarZ);
		glUniform1f(glGetUniformLocation(materialProgram, "f_minus_n"), lightFarZ - lightNearZ);
		glUniform1f(glGetUniformLocation(materialProgram, "f_mult_n"), lightFarZ * lightNearZ);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowTexture);
	}
	void OmnidirectionalShadow::deactivate() {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void OmnidirectionalShadow::addShaderCode(VertexShaderSource& vs, FragmentShaderSource& fs) {
		vs.setUseNormal(true);
		vs.outVar("vec3", "shadowCoord");
		vs.mainCode("vs_out.shadowCoord = vec3(modelTransform * vec4(position, 1));");

		fs.inVar("vec3", "normal");
		fs.inVar("vec3", "shadowCoord");
		fs.textureSamplerCubeShadow("depthSampler");
		fs.uniform("vec3", "shadowLightPos");
		fs.uniform("float", "f_plus_n");
		fs.uniform("float", "f_minus_n");
		fs.uniform("float", "f_mult_n");
		fs.mainCode("vec3 dir = fs_in.shadowCoord - shadowLightPos;");
		//fs.mainCode("float cosTheta = clamp(dot(normalize(fs_in.normal), -dir), 0, 1);");
		//fs.mainCode("float bias = clamp(0.05 * tan(acos(cosTheta)), 0, 0.1);");
		fs.mainCode("float localZ = max(abs(dir.x), max(abs(dir.y), abs(dir.z)));");
		fs.mainCode("localZ = f_plus_n/f_minus_n - (2*f_mult_n)/f_minus_n/localZ;");
		fs.mainCode("localZ = (localZ-0.004 + 1) * 0.5;");
		fs.mainCode("visibility = texture(depthSampler, vec4(normalize(dir), localZ));");
		fs.mainCode("if(visibility < .5) visibility = .5;");
	}

}