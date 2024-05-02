#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/render_command_list.h"
using namespace pathos;

namespace pathos {
	class OpenGLDevice;
	class PlaneGeometry;
	class Texture;
}

// Render skybox for world_rc1
class GalaxyGenerator {

public:
	static void renderStarField(Texture* texture, uint32 width, uint32 height, float dustIntensity);

	static void internal_createResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);
	static void internal_destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList);

private:
	static GLuint dummyFBO;

};
