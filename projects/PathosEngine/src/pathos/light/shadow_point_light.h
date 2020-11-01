#pragma once

#include "gl_core.h"
#include "pathos/camera/camera.h"
#include "pathos/scene/scene.h"
#include "pathos/shader/uniform_buffer.h"

#include "badger/types/noncopyable.h"
#include <vector>

namespace pathos {

	// Shadow pass for point lights.
	class PointLightShadowPass : public Noncopyable {
		static const uint32 SHADOW_MAP_SIZE;

	public:
		PointLightShadowPass() = default;
		virtual ~PointLightShadowPass() = default;

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);
		void renderShadowMaps(RenderCommandList& cmdList, const Scene* scene, const Camera* camera);

	private:
		GLuint fbo = 0;
		UniformBuffer ubo;
	};

}
