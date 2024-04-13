#include "buffer.h"

#include "pathos/rhi/render_device.h"
#include "pathos/util/log.h"

namespace pathos {

	Buffer::~Buffer() {
		releaseGPUResource();
	}

	void Buffer::createGPUResource(bool flushGPU /*= false*/) {
		if (created) {
			LOG(LogWarning, "Buffer %s was already created", createParams.debugName.size() > 0 ? createParams.debugName : "<noname>");
			return;
		}
		created = true;

		auto This = this;
		ENQUEUE_RENDER_COMMAND(
			[This](RenderCommandList& cmdList) {
				if (This->glBuffer != 0) {
					gRenderDevice->deleteTextures(1, &This->glBuffer);
					This->glBuffer = 0;
				}
				gRenderDevice->createBuffers(1, &This->glBuffer);

				const BufferCreateParams& createParams = This->getCreateParams();
				const GLuint glBuffer = This->glBuffer;

				// #todo-rhi: Persistent mapping instead of GL_DYNAMIC_STORAGE_BIT
				// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferStorage.xhtml
				GLenum usage = 0;
				if (ENUM_HAS_FLAG(createParams.usage, EBufferUsage::CpuWrite)) usage |= GL_DYNAMIC_STORAGE_BIT;

				cmdList.namedBufferStorage(glBuffer, createParams.bufferSize, createParams.initialData, usage);
				if (createParams.debugName.size() > 0) {
					cmdList.objectLabel(GL_BUFFER, glBuffer, -1, createParams.debugName.c_str());
				}
			}
		); // ENQUEUE_RENDER_COMMAND
		if (flushGPU) {
			FLUSH_RENDER_COMMAND(true);
		}
	}

	void Buffer::releaseGPUResource() {
		if (glBuffer != 0) {
			ENQUEUE_DEFERRED_RENDER_COMMAND([buf = glBuffer](RenderCommandList& cmdList) {
				cmdList.registerDeferredBufferCleanup(buf);
			});
			glBuffer = 0;
		}
	}

	void Buffer::writeToGPU(int64 offset, int64 size, void* data) {
		// #todo-performance: unnecessary copy
		auto This = this;
		void* data2 = std::malloc(size);
		CHECK(data2 != 0);
		std::memcpy(data2, data, size);
		ENQUEUE_RENDER_COMMAND([This, size, offset, data2](RenderCommandList& cmdList) {
			This->writeToGPU_renderThread(cmdList, offset, size, data2);
			std::free(data2);
		});
	}

	void Buffer::writeToGPU_renderThread(RenderCommandList& cmdList, int64 offset, int64 size, void* data) {
		CHECK(isInRenderThread());
		CHECK(size + offset <= createParams.bufferSize);
		CHECK(glBuffer != 0);
		CHECK(ENUM_HAS_FLAG(createParams.usage, EBufferUsage::CpuWrite));
		cmdList.namedBufferSubData(glBuffer, offset, size, data);
	}

	void Buffer::bindAsUBO(RenderCommandList& cmdList, GLuint bindingIndex) {
		cmdList.bindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, glBuffer);
	}

	void Buffer::bindAsSSBO(RenderCommandList& cmdList, GLuint bindingIndex) {
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, glBuffer);
	}

}

namespace pathos {

	BufferPool::~BufferPool() {
		releaseGPUResource();
	}

	void BufferPool::createGPUResource(uint64 totalBytes, const char* debugName, bool flushGPU /*= false*/) {
		CHECK(internalBuffer == nullptr);
		if (internalBuffer == nullptr) {
			CHECKF(totalBytes <= 0xffffffff, "totalBytes can't exceed uint32 max yet... need to refactor Buffer class");
			BufferCreateParams createParams{ EBufferUsage::CpuWrite, (uint32)totalBytes, nullptr, debugName };
			internalBuffer = new Buffer(createParams);
			internalBuffer->createGPUResource(flushGPU);
			mallocEmulator.initialize(totalBytes);
		}
	}

	void BufferPool::releaseGPUResource() {
		if (internalBuffer != nullptr) {
			internalBuffer->releaseGPUResource();
			delete internalBuffer;
			mallocEmulator.cleanup();
		}
	}

	void BufferPool::writeToGPU(int64 offset, int64 size, void* data) {
		internalBuffer->writeToGPU(offset, size, data);
	}

	void BufferPool::writeToGPU_renderThread(RenderCommandList& cmdList, int64 offset, int64 size, void* data) {
		internalBuffer->writeToGPU_renderThread(cmdList, offset, size, data);
	}

	uint64 BufferPool::suballocate(uint64 bytes) {
		std::lock_guard<std::mutex> guard(allocMutex);
		return mallocEmulator.allocate(bytes);
	}

	void BufferPool::deallocate(uint64 offset) {
		std::lock_guard<std::mutex> guard(allocMutex);
		mallocEmulator.deallocate(offset);
	}

	GLuint BufferPool::internal_getGLName() const {
		return internalBuffer->internal_getGLName();
	}

}
