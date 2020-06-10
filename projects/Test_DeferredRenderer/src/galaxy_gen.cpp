#include "galaxy_gen.h"

#include "badger/assertion/assertion.h"

#include "pathos/render/render_device.h"
#include "pathos/shader/shader_program.h"
#include "pathos/thread/engine_thread.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_util.h"
using namespace pathos;

class StarfieldVS : public ShaderStage {
public:
	StarfieldVS() : ShaderStage(GL_VERTEX_SHADER, "StarfieldVS")
	{
		setFilepath("fullscreen_quad.glsl");
	}
};

class StarfieldFS : public ShaderStage {
public:
	StarfieldFS() : ShaderStage(GL_FRAGMENT_SHADER, "StarfieldFS")
	{
		setFilepath("starfield.glsl");
	}
};

DEFINE_SHADER_PROGRAM2(Program_Starfield, StarfieldVS, StarfieldFS);

//////////////////////////////////////////////////////////////////////////


GLuint GalaxyGenerator::dummyFBO = 0;
PlaneGeometry* GalaxyGenerator::fullscreenQuad = nullptr;

GLuint GalaxyGenerator::createStarField(uint32 width, uint32 height) {
	CHECK(isInMainThread());

	GLuint texture = 0;

	FLUSH_RENDER_COMMAND();

	ENQUEUE_RENDER_COMMAND([texturePtr = &texture, width, height](RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(RenderStarfield);

		GLuint fbo = dummyFBO;

		gRenderDevice->createTextures(GL_TEXTURE_2D, 1, texturePtr);
		cmdList.textureStorage2D(*texturePtr, 1, GL_RGBA16F, width, height);

		cmdList.viewport(0, 0, width, height);
		cmdList.disable(GL_DEPTH_TEST);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Starfield);
		cmdList.useProgram(program.getGLName());
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, *texturePtr, 0);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
	});

	FLUSH_RENDER_COMMAND();

	return texture;
}

void GalaxyGenerator::internal_createResources(OpenGLDevice* renderDevice) {
	RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

	gRenderDevice->createFramebuffers(1, &dummyFBO);
	cmdList.namedFramebufferDrawBuffer(dummyFBO, GL_COLOR_ATTACHMENT0);

	fullscreenQuad = new PlaneGeometry(2.0f, 2.0f);
}

void GalaxyGenerator::internal_destroyResources(OpenGLDevice* renderDevice) {
	RenderCommandList& cmdList = renderDevice->getImmediateCommandList();

	gRenderDevice->deleteFramebuffers(1, &dummyFBO);

	delete fullscreenQuad;
}

DEFINE_GLOBAL_RENDER_ROUTINE(GalaxyGenerator, GalaxyGenerator::internal_createResources, GalaxyGenerator::internal_destroyResources);
