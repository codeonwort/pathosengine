#pragma once

#include "gl_debug_group.h"
#include "render_command_list.h"
#include "pathos/smart_pointer.h"
#include "pathos/util/engine_thread.h"

#include "badger/types/noncopyable.h"
#include <functional>
#include <memory>
#include <set>

#define REQUIRED_GL_MAJOR_VERSION 4
#define REQUIRED_GL_MINOR_VERSION 6

namespace pathos {

	class BufferPool;

	// https://developer.nvidia.com/vulkan-turing
	struct OpenGLExtensionSupport {
		uint32 ATI_meminfo : 1;
		uint32 NV_ray_tracing : 1;
		uint32 NV_mesh_shader : 1;
		uint32 NV_shading_rate_image : 1;
		uint32 NV_shader_texture_footprint : 1;
		uint32 NV_representative_fragment_test : 1;
		uint32 NV_fragment_shader_barycentric : 1;
		uint32 NV_compute_shader_derivatives : 1;
		uint32 NV_scissor_exclusive : 1;
		uint32 NVX_gpu_memory_info : 1;
		uint32 NV_gpu_shader5 : 1;
		uint32 EXT_shader_16bit_storage : 1;
		uint32 EXT_shader_explicit_arithmetic_types : 1;
	};

	struct OpenGLDriverCapabilities {
		int32 glObjectLabelMaxLength;
		// The maximum size in basic machine units of a uniform block, which must be at least 16384.
		int32 glMaxUniformBlockSize;
		// The number of invocations in a single local work group (i.e., the product of the three dimensions) that may be dispatched to a compute shader.
		int32 glMaxComputeWorkGroupInvocations;
		// Limit on the total storage size (in bytes) for all shared variables in a compute shader.
		int32 glMaxComputeSharedMemorySize;
		// The maximum number of work groups that may be dispatched to a compute shader.
		int32 glMaxComputeWorkGroupCount[3];
		// The maximum size of a work groups that may be used during compilation of a compute shader.
		int32 glMaxComputeWorkGroupSize[3];

		bool bMemoryInfoAvailable = false;
		int32 dedicatedVideoMemoryKiB = 0; // Total size of the VRAM
		int32 dedicatedVideoMemoryMiB = 0;
		int32 dedicatedVideoMemoryAvailableKiB = 0; // Available size of the VRAM
		int32 dedicatedVideoMemoryAvailableMiB = 0;
	};

	// Graphics device API wrapper
	class OpenGLDevice final : public Noncopyable {

	public:
		OpenGLDevice();
		~OpenGLDevice();

		bool initialize();

		void destroyGlobalResources();

		void memreport(int64& outTotalBufferMemory, int64& outTotalTextureMemory);
		void reportLiveObjects();

		const OpenGLExtensionSupport& getExtensionSupport() const { return extensionSupport; }
		const OpenGLDriverCapabilities& getCapabilities() const { return capabilities; }

		// #todo-renderthread: I messed it up :/ Can I unify them?
		__forceinline RenderCommandList& getImmediateCommandList() const { return *immediate_command_list.get(); }
		// Used for queueing commands from non-render threads.
		__forceinline RenderCommandList& getEarlyCommandList() const { return *early_command_list.get(); }
		__forceinline RenderCommandList& getDeferredCommandList() const { return *deferred_command_list.get(); }
		// Due to hooks appending commands at the rear of immediate list and messing up command order,
		// we pass a dedicated list to the current hook and immediately flush it. Dirty but at least does not fuck up the order.
		__forceinline RenderCommandList& getHookCommandList() const { return *hook_command_list.get(); }

	// API for GPU resource creation and deletion (not queued in command list)
	public:
		// Needed for texture view. Use createTextures() for normal case.
		void genTextures(GLsizei n, GLuint* textures);
		void genQueries(GLsizei n, GLuint* queries);

		void createVertexArrays(GLsizei n, GLuint* arrays);
		void createTextures(GLenum target, GLsizei n, GLuint* textures);
		void createFramebuffers(GLsizei n, GLuint* framebuffers);
		void createSamplers(GLsizei n, GLuint* samplers);
		void createQueries(GLenum target, GLsizei n, GLuint* ids);
		void createTransformFeedbacks(GLsizei n, GLuint* ids);
		void createBuffers(GLsizei n, GLuint* buffers);
		void createRenderbuffers(GLsizei n, GLuint* renderbuffers);
		void createProgramPipelines(GLsizei n, GLuint* pipelines);

		void deleteVertexArrays(GLsizei n, const GLuint* arrays);
		void deleteTextures(GLsizei n, const GLuint* textures);
		void deleteFramebuffers(GLsizei n, const GLuint* framebuffers);
		void deleteSamplers(GLsizei n, const GLuint* samplers);
		void deleteQueries(GLsizei n, const GLuint* ids);
		void deleteTransformFeedbacks(GLsizei n, const GLuint* ids);
		void deleteBuffers(GLsizei n, const GLuint* buffers);
		void deleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
		void deleteProgramPipelines(GLsizei n, const GLuint* pipelines);

		void objectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);

		inline BufferPool* getPositionBufferPool() const { return positionBufferPool; }
		inline BufferPool* getVaryingBufferPool() const { return varyingBufferPool; }
		inline BufferPool* getIndexBufferPool() const { return indexBufferPool; }
		inline GLuint getPositionOnlyVAO() const { return positionOnlyVAO; }

	private:
		void queryCapabilities();
		void checkExtensions();

		OpenGLDriverCapabilities capabilities;
		OpenGLExtensionSupport extensionSupport;

		uniquePtr<RenderCommandList> early_command_list;  // For render hooks in non-render threads
		uniquePtr<RenderCommandList> immediate_command_list; // For render thread itself
		uniquePtr<RenderCommandList> deferred_command_list;  // For render hooks in non-render threads
		uniquePtr<RenderCommandList> hook_command_list;

		BufferPool* positionBufferPool = nullptr; // Global position-only vertex buffer pool
		BufferPool* varyingBufferPool = nullptr; // Global non-position vertex buffer pool (normal, texcoord, ...)
		BufferPool* indexBufferPool = nullptr; // Global index buffer pool
		GLuint positionOnlyVAO = 0; // Global position-only vertex array object for indirect draw
	};

	extern OpenGLDevice* gRenderDevice;

}

namespace pathos {

	// Reserve a render command from the game thread. The command is executed BEFORE frame rendering.
	void ENQUEUE_RENDER_COMMAND(std::function<void(RenderCommandList& immediateCommandList)> lambda);

	// Reserve a render command from the game thread. The command is executed AFTER frame rendering.
	void ENQUEUE_DEFERRED_RENDER_COMMAND(std::function<void(RenderCommandList& immediateCommandList)> lambda);

	// Block the game thread until all render commands so far are finished.
	// CAUTION: Use only if must. Never use inside of game tick.
	void FLUSH_RENDER_COMMAND(bool waitForGPU = false);

}
