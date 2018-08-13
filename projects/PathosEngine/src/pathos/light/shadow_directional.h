#pragma once

#include <stdint.h>
#include <vector>
#include "GL/gl_core_4_3.h"

#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"

namespace pathos {

	// Shadow mapping by directional light. (usually Sun)
	// It's prepass prior to any shading works, so can be used in both forward and deferred renderers.
	class DirectionalShadowMap {

	public:
		DirectionalShadowMap(const glm::vec3& lightDirection = glm::vec3(0.0f, -1.0f, 0.0f));
		DirectionalShadowMap(const DirectionalShadowMap& other) = delete;
		DirectionalShadowMap(DirectionalShadowMap&& other) = delete;
		virtual ~DirectionalShadowMap();

		void setLightDirection(const glm::vec3& direction);
		void renderShadowMap(const Scene* scene, const Camera* camera);

		inline GLuint getDepthMapTexture() const { return depthTexture; }
		inline glm::mat4 getViewProjection(uint32_t index) const { return viewProjection[index]; }

	private:
		GLuint fbo;
		GLuint depthTexture;

		uint32_t numCascades;
		GLsizei depthMapWidth;
		GLsizei depthMapHeight; // depth map resolution

		GLuint program; // shadow mapping
		GLint uniform_depthMVP = -1;
		std::vector<GLint> textureBindings;

		// light space transform
		glm::vec3 lightDirection;
		//glm::mat4 view;
		//glm::mat4 projection;
		std::vector<glm::mat4> viewProjection;
	};

}