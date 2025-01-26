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

		inline matrix4 getViewProjection(uint32 lightIndex, uint32 cascadeIndex) const { return lightTransforms[lightIndex].viewProjectionMatrices[cascadeIndex]; }
		inline float getZSlice(uint32 lightIndex, uint32 cascadeIx) const { return lightTransforms[lightIndex].zSlices[cascadeIx]; }

	private:
		void calculateBounds(size_t lightIx, size_t cascadeIx, const vector3* frustum, const vector3& lightDir);

	private:
		bool bDestroyed = false;

		GLuint fbo = 0xffffffff;
		UniformBuffer uboPerFrame;
		UniformBuffer uboPerObject;

		// light space transform
		struct LightTransform {
			std::vector<matrix4> viewMatrices; // Light view matrices
			std::vector<matrix4> viewProjectionMatrices; // ViewProj matrices that perfectly cover each camera frustum
			std::vector<float> zSlices;
		};
		std::vector<LightTransform> lightTransforms;
	};

}
