#pragma once

#include "badger/types/noncopyable.h"

#include "render_command_list.h"
#include "pathos/thread/engine_thread.h"

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
	};

	class OpenGLDevice final : public Noncopyable {

	public:
		OpenGLDevice();
		~OpenGLDevice();

		bool initialize();

		const OpenGLExtensionSupport& getExtensionSupport() const { return extensionSupport; }
		// #todo-renderthread-fatal: I messed it up :/ Can I unify it?
		__forceinline RenderCommandList& getImmediateCommandList() const { return *immediate_command_list.get(); }
		__forceinline RenderCommandList& getDeferredCommandList() const { return *deferred_command_list.get(); }
		// Due to hooks appending commands at the rears of immediate/deferred and messing up command order,
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
		void deleteQueries(GLsizei n, const GLuint* ids);
		void deleteBuffers(GLsizei n, const GLuint* buffers);
		void deleteProgram(GLuint program);

		void objectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);

		GLint getUniformLocation(GLuint program, const GLchar* name);

	private:
		void checkExtensions();

		OpenGLExtensionSupport             extensionSupport;
		std::unique_ptr<RenderCommandList> immediate_command_list; // For render thread itself
		std::unique_ptr<RenderCommandList> deferred_command_list;  // For render hooks in non-render threads
		std::unique_ptr<RenderCommandList> hook_command_list;

		int32                              glObjectLabelMaxLength;
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
