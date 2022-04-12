#pragma once

// #todo-persistent-map-buffer
// https://www.bfilipek.com/2015/01/persistent-mapped-buffers-in-opengl.html

#include "pathos/render/render_device.h"
#include "badger/assertion/assertion.h"
#include "gl_core.h"

namespace pathos {

	class UniformBuffer {

	public:
		UniformBuffer()
			: ubo(0)
			, bufferSize(0)
		{
		}

		~UniformBuffer() {
			safeDestroy();
		}

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;

		template<typename T> void init() {
			//bufferSize = (sizeof(T) + 255) & ~255;
			bufferSize = sizeof(T);
			gRenderDevice->createBuffers(1, &ubo);
			// #todo-renderthread
			if (isInRenderThread()) {
				RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
				cmdList.namedBufferStorage(this->ubo, this->bufferSize, (GLvoid*)0, GL_DYNAMIC_STORAGE_BIT);
			} else {
				ENQUEUE_RENDER_COMMAND([this](RenderCommandList& cmdList) {
					cmdList.namedBufferStorage(this->ubo, this->bufferSize, (GLvoid*)0, GL_DYNAMIC_STORAGE_BIT);
				});
			}
		}

		void update(RenderCommandList& cmdList, GLuint bindingIndex, void* data) {
			CHECK(ubo);
			cmdList.namedBufferSubData(ubo, 0, bufferSize, data);
			cmdList.bindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ubo);
		}

		// NOTE: No need to call manually if this instance is deallocated before gRenderDevice shutdown.
		void safeDestroy() {
			if (ubo != 0) {
				gRenderDevice->deleteBuffers(1, &ubo);
				ubo = 0;
			}
		}

	private:
		GLuint ubo;
		uint32 bufferSize;

	};

}
