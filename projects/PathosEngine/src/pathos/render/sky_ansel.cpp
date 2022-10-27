#include "sky_ansel.h"
#include "render_device.h"
#include "pathos/engine_policy.h"
#include "pathos/scene/sky_ansel_component.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/shader/shader_program.h"
#include "pathos/util/gl_debug_group.h"

#include <string>

namespace pathos {
	
	class AnselSkyVS : public ShaderStage {
	public:
		AnselSkyVS() : ShaderStage(GL_VERTEX_SHADER, "AnselSkyVS") {
			addDefine("VERTEX_SHADER", 1);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z", 1);
			}
			setFilepath("sky_ansel.glsl");
		}
	};

	class AnselSkyFS : public ShaderStage {
	public:
		AnselSkyFS() : ShaderStage(GL_FRAGMENT_SHADER, "AnselSkyVS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("sky_ansel.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_AnselSky, AnselSkyVS, AnselSkyFS);

}

namespace pathos {

	void AnselSkyPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_AnselSky");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		uniform_transform = 0;
	}

	void AnselSkyPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void AnselSkyPass::render(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(AnselSkyActor);

		const Camera& camera = scene->camera;
		AnselSkyProxy* anselSky = scene->anselSky;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
		const matrix4& proj = camera.getProjectionMatrix();
		const matrix4 transform = proj * view;

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AnselSky);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &transform[0][0]);
		cmdList.bindTextureUnit(0, anselSky->textureID);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		anselSky->sphere->activate_position(cmdList);
		anselSky->sphere->activateIndexBuffer(cmdList);
		anselSky->sphere->drawPrimitive(cmdList);
	}

}
