#pragma once

// https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object

#include "pathos/gl_handles.h"
#include "pathos/render/render_device.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	// #todo-shader: For now not used anywhere, not tested at all.
	class ShaderStorageBuffer {

	public:
		ShaderStorageBuffer()
			: ssbo(0)
			, bufferSize(0) {
		}

		~ShaderStorageBuffer() {
			safeDestroy();
		}

		ShaderStorageBuffer(const ShaderStorageBuffer&) = delete;
		ShaderStorageBuffer& operator=(const ShaderStorageBuffer&) = delete;

		template<typename T> void init() {
			//bufferSize = (sizeof(T) + 255) & ~255;
			bufferSize = sizeof(T);
			gRenderDevice->createBuffers(1, &ssbo);

			// #todo-shader: Support SSBO flags for CPU-side read/write?
			const GLbitfield flags = 0;

			if (isInRenderThread()) {
				RenderCommandList& cmdList = gRenderDevice->getImmediateCommandList();
				cmdList.namedBufferStorage(this->ssbo, this->bufferSize, (GLvoid*)0, flags);
			} else {
				// #todo-renderthread: No issue with order of commands?
				ENQUEUE_RENDER_COMMAND([this](RenderCommandList& cmdList) {
					cmdList.namedBufferStorage(this->ssbo, this->bufferSize, (GLvoid*)0, flags);
				});
			}
		}

		// #todo-shader: Support SSBO update by CPU?
		//void update(RenderCommandList& cmdList, GLuint bindingIndex, void* data) {
		//	CHECK(isInRenderThread());
		//	CHECK(ssbo != 0);
		//	cmdList.namedBufferSubData(ssbo, 0, bufferSize, data);
		//	cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo);
		//}

		// NOTE: No need to call manually if this instance is deallocated before gRenderDevice shutdown.
		void safeDestroy() {
			if (ssbo != 0) {
				gRenderDevice->deleteBuffers(1, &ssbo);
				ssbo = 0;
			}
		}

		inline GLuint getGLName() const { return ssbo; }

	private:
		GLuint ssbo;
		uint32 bufferSize;

	};

}
