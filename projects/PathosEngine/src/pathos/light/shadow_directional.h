#pragma once

#include "gl_core.h"
#include "pathos/camera/camera.h"
#include "pathos/scene/scene.h"

#include <vector>

namespace pathos {

	// Shadow mapping by directional light. (usually Sun)
	// It's prepass prior to any material shading, so can be used in both forward and deferred renderers.
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

		inline glm::mat4 getViewProjection(uint32_t index) const { return viewProjectionMatrices[index]; }

	private:
		struct CSM_MeshBatch {
			CSM_MeshBatch(class MeshGeometry* inGeometry, const glm::mat4& inModelMatrix)
				: geometry(inGeometry)
				, modelMatrix(inModelMatrix)
			{
			}
			class MeshGeometry* geometry;
			glm::mat4 modelMatrix;
		};
		void collectMeshBatches(const Scene* scene, std::vector<CSM_MeshBatch>& outMeshBatches, std::vector<CSM_MeshBatch>& outWireframeBatches);

		// Update viewProjectionMatrices
		void calculateBounds(const Camera& camera, uint32 numCascades);

	private:
		bool destroyed = false;

		GLuint fbo = 0xffffffff;

		GLuint program = 0xffffffff; // shadow mapping
		GLint uniform_depthMVP = -1;
		std::vector<GLint> textureBindings;

		// light space transform
		glm::vec3 lightDirection;
		std::vector<glm::mat4> viewProjectionMatrices;

	};

}
