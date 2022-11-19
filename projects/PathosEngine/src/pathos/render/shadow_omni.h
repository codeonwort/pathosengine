#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/uniform_buffer.h"
#include "pathos/scene/camera.h"

#include "badger/types/noncopyable.h"
#include <vector>

namespace pathos {

	// Shadow pass for point lights.
	class OmniShadowPass : public Noncopyable {
		static const uint32 SHADOW_MAP_SIZE;

	public:
		OmniShadowPass() = default;
		virtual ~OmniShadowPass() = default;

		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);
		void renderShadowMaps(RenderCommandList& cmdList, const SceneProxy* scene, const Camera* camera);

	private:
		GLuint fbo = 0;
		UniformBuffer ubo;
	};

}
