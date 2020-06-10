#pragma once

#include "gl_core.h"
using namespace pathos;

namespace pathos {
	class OpenGLDevice;
	class PlaneGeometry;
}

class GalaxyGenerator {

public:
	static GLuint createStarField(uint32 width, uint32 height);

	static void internal_createResources(OpenGLDevice* renderDevice);
	static void internal_destroyResources(OpenGLDevice* renderDevice);

private:
	static GLuint dummyFBO;
	static PlaneGeometry* fullscreenQuad;

};
