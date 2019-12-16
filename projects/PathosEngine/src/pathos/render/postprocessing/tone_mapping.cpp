#include "tone_mapping.h"
#include "pathos/console.h"
#include "pathos/shader/shader.h"
#include "pathos/render/scene_render_targets.h"

namespace pathos {

	static ConsoleVariable<float> cvar_tonemapping_exposure("r.tonemapping.exposure", 1.0f, "exposure parameter of tone mapping pass");
	static ConsoleVariable<float> cvar_gamma("r.gamma", 2.2f, "gamma correction");

	void ToneMapping::initializeResources(RenderCommandList& cmdList)
	{
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		std::string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;
void main() {
	gl_Position = vec4(position, 1.0);
}
)";

		Shader vs(GL_VERTEX_SHADER, "VS_ToneMapping");
		vs.setSource(vshader);

		Shader fs(GL_FRAGMENT_SHADER, "FS_ToneMapping");
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

		ConsoleVariableBase* cvar_dof = ConsoleVariableManager::find("r.dof.enable");
		if(cvar_dof && cvar_dof->getInt() != 0) {
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		} else {
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		cmdList.useProgram(program);

		UBO_ToneMapping uboData;
		uboData.exposure = cvar_tonemapping_exposure.getValue();
		uboData.gamma    = cvar_gamma.getValue();
		ubo.update(cmdList, 0, &uboData);

		GLuint tonemapping_attachments[] = { sceneContext.sceneColor, sceneContext.sceneBloom, sceneContext.godRayResult };
		cmdList.bindTextures(0, 3, tonemapping_attachments);

		fullscreenQuad->drawPrimitive(cmdList);
	}

}
