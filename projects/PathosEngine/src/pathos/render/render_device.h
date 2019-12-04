#pragma once

#include "render_command_list.h"

#include <functional>
#include <memory>

// Device API wrapper

#define REQUIRED_GL_MAJOR_VERSION 4
#define REQUIRED_GL_MINOR_VERSION 6

namespace pathos {

	class OpenGLDevice {
		
	public:
		OpenGLDevice();
		~OpenGLDevice();

		OpenGLDevice(const OpenGLDevice&&) = delete;
		OpenGLDevice& operator=(const OpenGLDevice&&) = delete;

		bool initialize();

		__forceinline RenderCommandList& getImmediateCommandList() const { return *immediate_command_list.get(); }

	private:
		std::unique_ptr<RenderCommandList> immediate_command_list;

	};

	extern OpenGLDevice* gRenderDevice;

	// For game thread
	inline void ENQUEUE_RENDER_COMMAND(std::function<void(RenderCommandList& immediateCommandList)> lambda) {
		lambda(gRenderDevice->getImmediateCommandList());
	}

}
