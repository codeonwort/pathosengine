#pragma once

// #todo-persistent-map-buffer
// https://www.bfilipek.com/2015/01/persistent-mapped-buffers-in-opengl.html

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
			if (ubo != 0) {
				glDeleteBuffers(1, &ubo);
			}
		}

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;

		template<typename T> void init() {
			//bufferSize = (sizeof(T) + 255) & ~255;
			bufferSize = sizeof(T);
			glCreateBuffers(1, &ubo);
			glNamedBufferStorage(ubo, bufferSize, (GLvoid*)0, GL_DYNAMIC_STORAGE_BIT);
		}

		void update(RenderCommandList& cmdList, GLuint bindingIndex, void* data) {
			CHECK(ubo);
			cmdList.namedBufferSubData(ubo, 0, bufferSize, data);
			cmdList.bindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ubo);
		}

	private:
		GLuint ubo;
		uint32 bufferSize;

	};

}
