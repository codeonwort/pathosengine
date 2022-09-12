#pragma once

#include "badger/types/noncopyable.h"

#include "render_command_list.h"
#include "pathos/thread/engine_thread.h"
#include "pathos/util/gl_debug_group.h"

#include <functional>
#include <memory>

// Device API wrapper

#define REQUIRED_GL_MAJOR_VERSION 4
#define REQUIRED_GL_MINOR_VERSION 6

#define GL_DEBUG_CONTEXT  0
#define GL_ERROR_CALLBACK 1

namespace pathos {

	// https://developer.nvidia.com/vulkan-turing
	struct OpenGLExtensionSupport {
		uint32 NV_ray_tracing : 1;
		uint32 NV_mesh_shader : 1;
		uint32 NV_shading_rate_image : 1;
		uint32 NV_shader_texture_footprint : 1;
		uint32 NV_representative_fragment_test : 1;
		uint32 NV_fragment_shader_barycentric : 1;
		uint32 NV_compute_shader_derivatives : 1;
		uint32 NV_scissor_exclusive : 1;
		uint32 NVX_gpu_memory_info : 1;
	};

	struct OpenGLDriverCapabilities {
		int32 glObjectLabelMaxLength;
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

	class OpenGLDevice final : public Noncopyable {

	public:
		OpenGLDevice();
		~OpenGLDevice();

		bool initialize();

		const OpenGLExtensionSupport& getExtensionSupport() const { return extensionSupport; }
		const OpenGLDriverCapabilities& getCapabilities() const { return capabilities; }

		// #todo-renderthread: I messed it up :/ Can I unify it?
		__forceinline RenderCommandList& getImmediateCommandList() const { return *immediate_command_list.get(); }
		// Used for queueing commands from non-render threads.
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

		void deleteTextures(GLsizei n, const GLuint* textures);
		void deleteVertexArrays(GLsizei n, const GLuint* arrays);
		void deleteFramebuffers(GLsizei n, const GLuint* framebuffers);
		void deleteSamplers(GLsizei n, const GLuint* samplers);
		void deleteQueries(GLsizei n, const GLuint* ids);
		void deleteBuffers(GLsizei n, const GLuint* buffers);
		void deleteProgram(GLuint program);

		void objectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);

		GLint getUniformLocation(GLuint program, const GLchar* name);

	private:
		void queryCapabilities();
		void checkExtensions();

		OpenGLDriverCapabilities           capabilities;
		OpenGLExtensionSupport             extensionSupport;

		std::unique_ptr<RenderCommandList> immediate_command_list; // For render thread itself
		std::unique_ptr<RenderCommandList> deferred_command_list;  // For render hooks in non-render threads
		std::unique_ptr<RenderCommandList> hook_command_list;
	};

	extern OpenGLDevice* gRenderDevice;

}

namespace pathos {

	// Reserve a render command from the game thread.
	void ENQUEUE_RENDER_COMMAND(std::function<void(RenderCommandList& immediateCommandList)> lambda);

	// Block the game thread until all render commands so far are finished.
	// CAUTION: Use only if must. Never use inside of game tick.
	void FLUSH_RENDER_COMMAND(bool waitForGPU = false);

	// #todo-renderthread: Flushes by this might be not needed after multithreading is properly implemented.
	void TEMP_FLUSH_RENDER_COMMAND(bool waitForGPU = false);

}
