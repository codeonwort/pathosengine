#pragma once

#include "gl_core.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/uniform_buffer.h"

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
		void destroyResources(RenderCommandList& cmdList);
		void renderShadowMaps(RenderCommandList& cmdList, const SceneProxy* scene, const Camera* camera);

	private:
		GLuint fbo = 0;
		UniformBuffer ubo;
	};

}
