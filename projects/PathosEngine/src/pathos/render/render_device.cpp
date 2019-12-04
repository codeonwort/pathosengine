#include "render_device.h"
#include "gl_core.h"
#include "pathos/util/log.h"

namespace pathos {

	OpenGLDevice* gRenderDevice = nullptr;

	OpenGLDevice::OpenGLDevice()
	{
		if (gRenderDevice)
		{
			assert(0);
		}
		gRenderDevice = this;
	}

	OpenGLDevice::~OpenGLDevice()
	{
	}

	bool OpenGLDevice::initialize()
	{
		if (gl3wInit()) {
			LOG(LogError, "Failed to initialize GL3W");
			return false;
		}
		if (!gl3wIsSupported(REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION)) {
			LOG(LogError, "GL %d.%d is not supported", REQUIRED_GL_MAJOR_VERSION, REQUIRED_GL_MINOR_VERSION);
			return false;
		}

		// #todo-renderdevice: move from engine.cpp

		LOG(LogInfo, "GL version: %s", glGetString(GL_VERSION));
		LOG(LogInfo, "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		return true;
	}

}
