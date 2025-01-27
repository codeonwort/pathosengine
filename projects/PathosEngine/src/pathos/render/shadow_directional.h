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

namespace pathos {

	// Generates shadow maps for directional lights (usually Sun).
	class DirectionalShadowMap final : public Noncopyable {

	public:
		virtual ~DirectionalShadowMap();

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);
		
		void renderShadowMap(RenderCommandList& cmdList, SceneProxy* scene, const Camera* camera, const UBO_PerFrame& cachedPerFrameUBOData);

	private:
		bool bDestroyed = false;

		GLuint fbo = 0xffffffff;
		UniformBuffer uboPerFrame;
		UniformBuffer uboPerObject;
	};

}
