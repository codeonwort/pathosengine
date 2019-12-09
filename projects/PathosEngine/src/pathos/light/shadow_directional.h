#pragma once

#include <vector>
#include "gl_core.h"

#include "pathos/camera/camera.h"
#include "pathos/render/scene.h"

namespace pathos {

	// Shadow mapping by directional light. (usually Sun)
	// It's prepass prior to any shading works, so can be used in both forward and deferred renderers.
	class DirectionalShadowMap {

	public:
		DirectionalShadowMap(const glm::vec3& lightDirection = glm::vec3(0.0f, -1.0f, 0.0f));
		virtual ~DirectionalShadowMap();

		DirectionalShadowMap(const DirectionalShadowMap&) = delete;
		DirectionalShadowMap& operator=(DirectionalShadowMap&) = delete;

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void setLightDirection(const glm::vec3& direction);
		
		void renderShadowMap(RenderCommandList& cmdList, const Scene* scene, const Camera* camera);

		inline glm::mat4 getViewProjection(uint32_t index) const { return viewProjection[index]; }


	private:
		bool destroyed = false;

		GLuint fbo;

		GLuint program; // shadow mapping
		GLint uniform_depthMVP = -1;
		std::vector<GLint> textureBindings;

		// light space transform
		glm::vec3 lightDirection;
		std::vector<glm::mat4> viewProjection;
	};

}
