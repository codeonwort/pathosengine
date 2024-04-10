#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/util/malloc_emulator.h"

#include "badger/types/noncopyable.h"
#include "badger/types/enum.h"
#include "badger/assertion/assertion.h"
#include <string>

namespace pathos {

	enum class EBufferUsage : uint32 {
		None     = 0,
		CpuWrite = 1 << 0, // CPU can write to buffer
	};
	ENUM_CLASS_FLAGS(EBufferUsage);

	struct BufferCreateParams {
		EBufferUsage usage     = EBufferUsage::None;
		uint32 bufferSize      = 0;
		void* initialData      = nullptr;
		std::string debugName;
	};

	/// <summary>
	/// Wrapper for GPU buffer resource.
	/// A Buffer instance maps to an individual VBO, so consider using BufferPool instead
	/// #wip: Replace UniformBuffer and vertex/index buffers with this?
	/// </summary>
	class Buffer final : public Noncopyable {

	public:
		Buffer(const BufferCreateParams& inCreateParams)
			: createParams(inCreateParams)
		{}

		~Buffer();

		void createGPUResource(bool flushGPU = false);
		void releaseGPUResource();

		void writeToGPU(int64 offset, int64 size, void* data);
		void writeToGPU_renderThread(RenderCommandList& cmdList, int64 offset, int64 size, void* data);

		void bindAsUBO(RenderCommandList& cmdList, GLuint bindingIndex);
		void bindAsSSBO(RenderCommandList& cmdList, GLuint bindingIndex);

		inline const BufferCreateParams& getCreateParams() const { return createParams; }
		inline GLuint internal_getGLName() const { return glBuffer; }

	private:
		const BufferCreateParams createParams;
		GLuint glBuffer = 0;
		bool created = false;
	};

	/// <summary>
	/// Create a single large buffer and suballocate it.
	/// #wip-bufferpool
	/// </summary>
	class BufferPool final : public Noncopyable {
		
	public:
		// Let suballocate() return a BufferView?
		//struct BufferView { uint64 offset, bytes; };
		static constexpr uint64 INVALID_OFFSET = MallocEmulator::INVALID_OFFSET;

		~BufferPool();

		void createGPUResource(uint64 totalBytes, const char* debugName, bool flushGPU = false);
		void releaseGPUResource();

		void writeToGPU(int64 offset, int64 size, void* data);
		void writeToGPU_renderThread(RenderCommandList& cmdList, int64 offset, int64 size, void* data);

		/// Returns offset. If failed, returns BufferPool::INVALID_OFFSET.
		uint64 suballocate(uint64 bytes);

		/// offset should be a value that was returned from suballocate().
		void deallocate(uint64 offset);

		GLuint internal_getGLName() const;

	private:
		Buffer* internalBuffer = nullptr;
		MallocEmulator mallocEmulator;

	};

}
