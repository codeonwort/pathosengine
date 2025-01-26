#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/scene/camera.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_renderer.h"

#include "badger/types/noncopyable.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include <vector>

namespace pathos {

	// Shadow pass for a directional light. (usually Sun)
	class DirectionalShadowMap final : public Noncopyable {

	public:
		virtual ~DirectionalShadowMap();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void updateUniformBufferData(RenderCommandList& cmdList, const SceneProxy* scene, const Camera* camera);
		
		void renderShadowMap(RenderCommandList& cmdList, SceneProxy* scene, const Camera* camera, const UBO_PerFrame& cachedPerFrameUBOData);

		inline matrix4 getViewProjection(uint32 index) const { return viewProjectionMatrices[index]; }

		float getShadowMapZFar() const;
		inline float getZSlice(uint32 ix) const { return zSlices[ix]; }

	private:
		void setLightDirection(const vector3& direction);
		void calculateBounds(const Camera& camera, uint32 numCascades, float zFar); // Update viewProjectionMatrices

	private:
		bool bDestroyed = false;

		GLuint fbo = 0xffffffff;
		UniformBuffer uboPerFrame;
		UniformBuffer uboPerObject;

		// light space transform
		vector3 lightDirection = vector3(0.0f, -1.0f, 0.0f);
		std::vector<matrix4> viewMatrices; // Light view matrices
		std::vector<matrix4> viewProjectionMatrices; // ViewProj matrices that perfectly cover each camera frustum
		float zSlices[4] = { 0.0f, };
		float zFar = 0.0f;
	};

}
