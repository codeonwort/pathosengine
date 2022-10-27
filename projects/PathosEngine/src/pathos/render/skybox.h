#pragma once

#include "pathos/gl_handles.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class SceneProxy;

	class SkyboxPass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;

	};

}
