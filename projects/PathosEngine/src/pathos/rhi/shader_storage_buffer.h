#pragma once

// https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_device.h"

#include "badger/types/noncopyable.h"
#include "badger/assertion/assertion.h"
#include <string>

namespace pathos {

	// #todo-shader: For now not used anywhere, not tested at all.
	class ShaderStorageBuffer final : public Noncopyable {

	public:
		ShaderStorageBuffer() {}
		~ShaderStorageBuffer() {
			safeDestroy();
		}

		void init(int64 inBufferSize, const char* inDebugName = nullptr) {
			bufferSize = inBufferSize;
			debugName = (inDebugName != nullptr) ? inDebugName : std::string();

			ENQUEUE_RENDER_COMMAND([this](RenderCommandList& cmdList) {
				// #todo-shader: Switch to AZDO version
				const GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;

				gRenderDevice->createBuffers(1, &ssbo);
				cmdList.namedBufferStorage(this->ssbo, this->bufferSize, (GLvoid*)0, flags);
				if (debugName.size() > 0) {
					cmdList.objectLabel(GL_BUFFER, this->ssbo, -1, this->debugName.c_str());
				}
			});
		}

		void update(RenderCommandList& cmdList, GLuint bindingIndex, void* srcData, GLsizeiptr srcSize) {
			CHECK(isInRenderThread());
			CHECK(ssbo != 0);
			cmdList.namedBufferSubData(ssbo, 0, srcSize, srcData);
			cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo);
		}

		void safeDestroy() {
			if (ssbo != 0) {
				ENQUEUE_RENDER_COMMAND([ssbo = this->ssbo](RenderCommandList& cmdList) {
					gRenderDevice->deleteBuffers(1, &ssbo);
				});
			}
		}

		inline GLuint getGLName() const { return ssbo; }

	private:
		GLuint ssbo = 0;
		int64 bufferSize = 0;
		std::string debugName;
	};

}
