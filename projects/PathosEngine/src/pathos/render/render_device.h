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

	class OpenGLDevice final : public Noncopyable {
		
	public:
		OpenGLDevice();
		~OpenGLDevice();

		bool initialize();

		__forceinline RenderCommandList& getImmediateCommandList() const { return *immediate_command_list.get(); }
		__forceinline RenderCommandList& getCommandListForHook() const { return *temp_command_list.get(); }

	// API for GPU resource creation and deletion (not queued in command list)
	public:
		void createVertexArrays(GLsizei n, GLuint* arrays);
		void createTextures(GLenum target, GLsizei n, GLuint* textures);
		void createFramebuffers(GLsizei n, GLuint* framebuffers);
		void createSamplers(GLsizei n, GLuint* samplers);
		void createQueries(GLenum target, GLsizei n, GLuint* ids);
		void createTransformFeedbacks(GLsizei n, GLuint* ids);
		void createBuffers(GLsizei n, GLuint* buffers);
		void createRenderbuffers(GLsizei n, GLuint* renderbuffers);
		void createProgramPipelines(GLsizei n, GLuint* pipelines);

	private:
		std::unique_ptr<RenderCommandList> immediate_command_list;
		std::unique_ptr<RenderCommandList> temp_command_list;

	};

	extern OpenGLDevice* gRenderDevice;

	// For game thread
	inline void ENQUEUE_RENDER_COMMAND(std::function<void(RenderCommandList& immediateCommandList)> lambda) {
		CHECK(isInMainThread());

		gRenderDevice->getImmediateCommandList().registerHook([lambda](void* param) -> void
			{
				RenderCommandList& tempCmdList = gRenderDevice->getCommandListForHook();
				lambda(tempCmdList);
				tempCmdList.flushAllCommands();
			}
		, nullptr, 0);
	}

	inline void FLUSH_RENDER_COMMAND() {
		CHECK(isInMainThread());

		gRenderDevice->getImmediateCommandList().flushAllCommands();
	}

}
