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
		setFilepath("sky/starfield.glsl");
	}
};

DEFINE_SHADER_PROGRAM2(Program_Starfield, FullscreenVS, StarfieldFS);

//////////////////////////////////////////////////////////////////////////

GLuint GalaxyGenerator::dummyFBO = 0;

void GalaxyGenerator::renderStarField(Texture* texture, uint32 width, uint32 height, float dustIntensity) {
	CHECK(isInMainThread());

	ENQUEUE_RENDER_COMMAND([texture, width, height, dustIntensity](RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(RenderStarfield);

		GLuint fbo = dummyFBO;
		GLuint renderTarget = texture->internal_getGLName();
		auto fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		// Set shader program.
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Starfield);
		cmdList.useProgram(program.getGLName());

		// Set render target.
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, renderTarget, 0);

		// Set resource bindings.
		cmdList.uniform1f(1, dustIntensity);

		// Set render states.
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.disable(GL_BLEND);
		cmdList.viewport(0, 0, width, height);

		// Drawcall.
		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
	});
}

void GalaxyGenerator::internal_createResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
	gRenderDevice->createFramebuffers(1, &dummyFBO);
	cmdList.namedFramebufferDrawBuffer(dummyFBO, GL_COLOR_ATTACHMENT0);
}

void GalaxyGenerator::internal_destroyResources(OpenGLDevice* renderDevice, RenderCommandList& cmdList) {
	gRenderDevice->deleteFramebuffers(1, &dummyFBO);
}

DEFINE_GLOBAL_RENDER_ROUTINE(GalaxyGenerator, GalaxyGenerator::internal_createResources, GalaxyGenerator::internal_destroyResources);
