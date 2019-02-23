#include "shadow_directional.h"
#include "pathos/util/log.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/mesh.h"
#include "glm/gtc/matrix_transform.hpp"

namespace pathos {

	DirectionalShadowMap::DirectionalShadowMap(const glm::vec3& lightDirection_) {
		lightDirection = lightDirection_;

		numCascades    = 4;
		depthMapWidth  = 2048;
		depthMapHeight = 2048;

		// create framebuffer object
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glObjectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_CascadedShadowMap");

		glDrawBuffers(0, nullptr);

		// setup depth textures
		glCreateTextures(GL_TEXTURE_2D, 1, &depthTexture);
// 		glGenTextures(1, &depthTexture);
// 		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glObjectLabel(GL_TEXTURE, depthTexture, -1, "Tex_CascadedShadowMap");
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, numCascades * depthMapWidth, depthMapHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// check if our framebuffer is ok
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			LOG(LogFatal, "Cannot create a framebuffer for shadow map");
			assert(0);
		}

		// return to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// create shadow program
		string vshader = R"(#version 430 core
layout (location = 0) in vec3 position;
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

		Shader vs(GL_VERTEX_SHADER, "VS_CascadedShadowMap");
		Shader fs(GL_FRAGMENT_SHADER, "FS_CascadedShadowMap");
		vs.setSource(vshader);
		fs.setSource(fshader);

		program = pathos::createProgram(vs, fs, "Program_CascadedShadowMap");

		uniform_depthMVP = glGetUniformLocation(program, "depthMVP");
		assert(uniform_depthMVP != -1);
	}

	DirectionalShadowMap::~DirectionalShadowMap() {
		glDeleteTextures(1, &depthTexture);
		glDeleteFramebuffers(1, &fbo);
		glDeleteProgram(program);
	}

	void DirectionalShadowMap::setLightDirection(const glm::vec3& direction) {
		lightDirection = direction;
	}

	void DirectionalShadowMap::renderShadowMap(const Scene* scene, const Camera* camera) {
		SCOPED_DRAW_EVENT(CascadedShadowMap);

		// 1. clear depth map
		static const GLfloat clear_depth_one[] = { 1.0f };
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearBufferfv(GL_DEPTH, 0, clear_depth_one);

		// 2. build projection matrix that perfectly covers camera frustum
		if (scene->directionalLights.size() > 0) {
			setLightDirection(scene->directionalLights[0]->getDirection());
		}

		auto calcBounds = [this](const glm::vec3* frustum) -> void {
			glm::vec3 sun_origin(0.0f, 0.0f, 0.0f);
			glm::vec3 sun_direction = lightDirection;
			glm::vec3 sun_up(0.0f, 1.0f, 0.0f);

			// if almost parallel, choose another random direction
			float angle = fabs(glm::dot(sun_up, sun_direction));
			if (angle >= 0.999f || angle <= 0.001f) {
				sun_up = glm::vec3(0.0f, 0.0f, -1.0f);
			}

			glm::mat4 lightView = glm::lookAt(sun_origin, sun_direction, sun_up);

			float minZ = std::numeric_limits<float>::max();
			float maxZ = std::numeric_limits<float>::min();
			float left = std::numeric_limits<float>::max();
			float right = std::numeric_limits<float>::min();
			float top = std::numeric_limits<float>::max();
			float bottom = std::numeric_limits<float>::min();

			for (int i = 0; i < 8; ++i) {
				glm::vec3 fv = glm::vec3(lightView * glm::vec4(frustum[i], 1.0f));
				if (fv.x < left) left = fv.x;
				if (fv.x > right) right = fv.x;
				if (fv.y < top) top = fv.y;
				if (fv.y > bottom) bottom = fv.y;
				if (fv.z < minZ) minZ = fv.z;
				if (fv.z > maxZ) maxZ = fv.z;
			}

			sun_origin += sun_direction * minZ;
			// get row of lightView
			glm::vec3 sun_right = glm::vec3(lightView[0][0], lightView[1][0], lightView[2][0]);
			sun_up = glm::vec3(lightView[0][1], lightView[1][1], lightView[2][1]);
			sun_origin -= ((left + right) / 2.0f) * sun_right;
			sun_origin -= ((top + bottom) / 2.0f) * sun_up;
			lightView = glm::lookAt(sun_origin, sun_origin + sun_direction, sun_up);

			glm::mat4 projection = glm::ortho(left, right, top, bottom, 0.0f, maxZ - minZ);
			viewProjection.emplace_back(projection * lightView);
		};

		std::vector<glm::vec3> frustum;
		camera->getFrustum(frustum, numCascades);
		viewProjection.clear();
		for (auto i = 0u; i < numCascades; ++i) {
			calcBounds(&frustum[i * 4]);
		}

		// 3. render depth map
		glUseProgram(program);
		glEnable(GL_DEPTH_TEST);
		
		for (auto i = 0u; i < numCascades; ++i) {
			glViewport(i * depthMapWidth, 0, depthMapWidth, depthMapHeight);
			const glm::mat4& VP = viewProjection[i];

			for (Mesh* mesh : scene->meshes) {
				const auto geometries = mesh->getGeometries();
				const auto materials = mesh->getMaterials();
				int ix = 0;
				for (const auto G : geometries) {
					glm::mat4 mvp = VP * mesh->getTransform().getMatrix();
					glUniformMatrix4fv(uniform_depthMVP, 1, GL_FALSE, &(mvp[0][0]));

					bool wasWireframe = false;
					if (materials[ix++]->getMaterialID() == MATERIAL_ID::WIREFRAME) {
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						wasWireframe = true;
					}

					G->activate_position();
					G->activateIndexBuffer();
					G->draw();
					G->deactivate();
					G->deactivateIndexBuffer();

					if (wasWireframe) {
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}
				}
			}
		}
	}

}
