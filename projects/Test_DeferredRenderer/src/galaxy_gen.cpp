#include "galaxy_gen.h"

#include "badger/assertion/assertion.h"

#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/texture.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/engine_thread.h"
#include "pathos/util/engine_util.h"
using namespace pathos;

class StarfieldFS : public ShaderStage {
public:
	StarfieldFS() : ShaderStage(GL_FRAGMENT_SHADER, "StarfieldFS")
	{
		setFilepath("starfield.glsl");
	}
};

DEFINE_SHADER_PROGRAM2(Program_Starfield, FullscreenVS, StarfieldFS);

//////////////////////////////////////////////////////////////////////////

GLuint GalaxyGenerator::dummyFBO = 0;
PlaneGeometry* GalaxyGenerator::fullscreenQuad = nullptr;

void GalaxyGenerator::renderStarField(Texture* texture, uint32 width, uint32 height) {
	CHECK(isInMainThread());

	ENQUEUE_RENDER_COMMAND([texture, width, height](RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(RenderStarfield);

		GLuint fbo = dummyFBO;
		GLuint renderTarget = texture->internal_getGLName();

		cmdList.viewport(0, 0, width, height);
		cmdList.disable(GL_DEPTH_TEST);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Starfield);
		cmdList.useProgram(program.getGLName());
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, renderTarget, 0);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
	});
}

void GalaxyGenerator::internal_createResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
	gRenderDevice->createFramebuffers(1, &dummyFBO);
	cmdList.namedFramebufferDrawBuffer(dummyFBO, GL_COLOR_ATTACHMENT0);

	fullscreenQuad = new PlaneGeometry(2.0f, 2.0f);
}

void GalaxyGenerator::internal_destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
	gRenderDevice->deleteFramebuffers(1, &dummyFBO);

	delete fullscreenQuad;
	fullscreenQuad = nullptr;
}

DEFINE_GLOBAL_RENDER_ROUTINE(GalaxyGenerator, GalaxyGenerator::internal_createResources, GalaxyGenerator::internal_destroyResources);
