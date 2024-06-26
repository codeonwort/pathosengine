#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/render_command_list.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	// #todo-rhi: Replace with Buffer
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

		void init(uint32 inBufferSize, const char* inDebugName = nullptr) {
			CHECK(inBufferSize > 0);
			bufferSize = inBufferSize;
			if (inDebugName != nullptr) {
				debugName = inDebugName;
			}

			if (isInRenderThread()) {
				gRenderDevice->createBuffers(1, &ubo);
				RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
				cmdList.namedBufferStorage(this->ubo, this->bufferSize, (GLvoid*)0, GL_DYNAMIC_STORAGE_BIT);
				if (inDebugName != nullptr) {
					cmdList.objectLabel(GL_BUFFER, this->ubo, -1, debugName.c_str());
				}
			} else {
				ENQUEUE_RENDER_COMMAND([this](RenderCommandList& cmdList) {
					gRenderDevice->createBuffers(1, &(this->ubo));
					cmdList.namedBufferStorage(this->ubo, this->bufferSize, (GLvoid*)0, GL_DYNAMIC_STORAGE_BIT);
					if (this->debugName.size() > 0) {
						cmdList.objectLabel(GL_BUFFER, this->ubo, -1, this->debugName.c_str());
					}
				});
			}
		}

		template<typename T> void init(const char* inDebugName = nullptr) {
			//bufferSize = (sizeof(T) + 255) & ~255;
			init(sizeof(T), inDebugName);
		}

		void update(RenderCommandList& cmdList, GLuint bindingIndex, void* data) {
			CHECK(isInRenderThread());
			CHECK(ubo != 0);
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
		std::string debugName;

	};

}
