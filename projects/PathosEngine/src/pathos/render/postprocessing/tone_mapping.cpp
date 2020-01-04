#include "tone_mapping.h"
#include "pathos/console.h"
#include "pathos/shader/shader.h"
#include "pathos/render/scene_render_targets.h"

namespace pathos {

	static ConsoleVariable<float> cvar_tonemapping_exposure("r.tonemapping.exposure", 1.2f, "exposure parameter of tone mapping pass");
	static ConsoleVariable<float> cvar_gamma("r.gamma", 2.2f, "gamma correction");

	void ToneMapping::initializeResources(RenderCommandList& cmdList)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		Shader vs(GL_VERTEX_SHADER, "VS_ToneMapping");
		Shader fs(GL_FRAGMENT_SHADER, "FS_ToneMapping");
		vs.loadSource("fullscreen_quad.glsl");
		fs.loadSource("tone_mapping.glsl");

		program = pathos::createProgram(vs, fs, "ToneMapping");
		ubo.init<UBO_ToneMapping>();

		// tone mapping resource
		cmdList.createFramebuffers(1, &fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.toneMappingResult, 0);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		checkFramebufferStatus(cmdList, fbo);
	}

	void ToneMapping::releaseResources(RenderCommandList& cmdList)
	{
		cmdList.deleteProgram(program);
		cmdList.deleteFramebuffers(1, &fbo);
	}

	void ToneMapping::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(ToneMapping);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		cmdList.useProgram(program);

		UBO_ToneMapping uboData;
		uboData.exposure = cvar_tonemapping_exposure.getValue();
		uboData.gamma    = cvar_gamma.getValue();
		ubo.update(cmdList, 0, &uboData);

		GLuint godRayTexture = sceneContext.godRayResult;
		ConsoleVariableBase* godray_upsampling = ConsoleVariableManager::find("r.godray.upsampling");
		if (godray_upsampling && godray_upsampling->getInt() != 0) {
			godRayTexture = sceneContext.godRayResultFiltered;
		}

		GLuint tonemapping_attachments[] = { sceneContext.sceneColor, sceneContext.sceneBloom, godRayTexture };
		cmdList.bindTextures(0, 3, tonemapping_attachments);

		fullscreenQuad->drawPrimitive(cmdList);
	}

}
