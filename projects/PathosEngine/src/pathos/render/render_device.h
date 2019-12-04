#pragma once

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

	};

	extern OpenGLDevice* gRenderDevice;

}
