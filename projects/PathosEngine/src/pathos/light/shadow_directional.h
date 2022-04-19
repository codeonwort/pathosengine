#pragma once

#include "gl_core.h"
#include "pathos/camera/camera.h"
#include "pathos/render/scene_proxy.h"

#include "badger/types/noncopyable.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include <vector>

namespace pathos {

	// Shadow pass for a directional light. (usually Sun)
	class DirectionalShadowMap : public Noncopyable {

	public:
		DirectionalShadowMap(const vector3& lightDirection = vector3(0.0f, -1.0f, 0.0f));
		virtual ~DirectionalShadowMap();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void updateUniformBufferData(RenderCommandList& cmdList, const SceneProxy* scene, const Camera* camera);
		
		void renderShadowMap(RenderCommandList& cmdList, SceneProxy* scene, const Camera* camera);

		inline matrix4 getViewProjection(uint32 index) const { return viewProjectionMatrices[index]; }

	private:
		void setLightDirection(const vector3& direction);
		// Update viewProjectionMatrices
		void calculateBounds(const Camera& camera, uint32 numCascades);

	private:
		bool destroyed = false;

		GLuint fbo = 0xffffffff;

		GLuint program = 0xffffffff; // shadow mapping
		GLint uniform_depthMVP = -1;

		// light space transform
		vector3 lightDirection;
		std::vector<matrix4> viewProjectionMatrices; // Projection matrices that perfectly cover each camera frustum

	};

}
