#pragma once

#include "pathos/gl_handles.h"
#include "pathos/shader/uniform_buffer.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	class SceneProxy;

	class AnselSkyPass : public Noncopyable {
		
	public:
		void initializeResources(RenderCommandList& cmdList);
		void releaseResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;

	};

}
