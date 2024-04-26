#include "auto_exposure.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/mesh/geometry.h"
#include "pathos/util/engine_util.h"
#include "pathos/engine.h"

namespace pathos {

	class AutoExposureFS : public ShaderStage {
	public:
		AutoExposureFS() : ShaderStage(GL_FRAGMENT_SHADER, "AutoExposureFS") {
			setFilepath("auto_exposure.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_AutoExposure, FullscreenVS, AutoExposureFS);

}

namespace pathos {

	AutoExposurePass::AutoExposurePass() {
	}

	AutoExposurePass::~AutoExposurePass() {
		CHECK(destroyed);
	}

	void AutoExposurePass::initializeResources(RenderCommandList& cmdList) {
		fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
	}

	void AutoExposurePass::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		destroyed = true;
	}

	void AutoExposurePass::renderAutoExposure(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(AutoExposure);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AutoExposure);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneLuminance, 0);

		pathos::checkFramebufferStatus(cmdList, fbo, "[AutoExposure] FBO is invalid");

		// Set render states
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.disable(GL_BLEND);

		cmdList.viewport(0, 0, sceneContext.sceneLuminanceSize, sceneContext.sceneLuminanceSize);

		cmdList.bindTextureUnit(0, sceneContext.sceneColor);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		// Release resource bindings
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		cmdList.bindTextureUnit(0, 0);

		// Generate mips
		cmdList.generateTextureMipmap(sceneContext.sceneLuminance);
	}

}
