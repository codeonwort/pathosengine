#pragma once

#include "gl_core.h"
#include "pathos/camera/camera.h"
#include "pathos/scene/scene.h"

#include "badger/types/noncopyable.h"
#include <vector>

namespace pathos {

	// Shadow pass for a directional light. (usually Sun)
	class DirectionalShadowMap : public Noncopyable {

	public:
		DirectionalShadowMap(const glm::vec3& lightDirection = glm::vec3(0.0f, -1.0f, 0.0f));
		virtual ~DirectionalShadowMap();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void setLightDirection(const glm::vec3& direction);
		
		void renderShadowMap(RenderCommandList& cmdList, const Scene* scene, const Camera* camera);

		inline glm::mat4 getViewProjection(uint32_t index) const { return viewProjectionMatrices[index]; }

	private:
		// Update viewProjectionMatrices
		void calculateBounds(const Camera& camera, uint32 numCascades);

	private:
		bool destroyed = false;

		GLuint fbo = 0xffffffff;

		GLuint program = 0xffffffff; // shadow mapping
		GLint uniform_depthMVP = -1;

		// light space transform
		glm::vec3 lightDirection;
		std::vector<glm::mat4> viewProjectionMatrices;

	};

}
