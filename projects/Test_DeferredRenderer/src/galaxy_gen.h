#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"
using namespace pathos;

namespace pathos {
	class OpenGLDevice;
	class PlaneGeometry;
	class Texture;
}

class GalaxyGenerator {

public:
	static void renderStarField(Texture* texture, uint32 width, uint32 height);

	static void internal_createResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
	static void internal_destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);

private:
	static GLuint dummyFBO;
	static PlaneGeometry* fullscreenQuad;

};
