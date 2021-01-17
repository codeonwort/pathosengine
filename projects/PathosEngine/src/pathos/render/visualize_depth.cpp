#include "visualize_depth.h"
#include "scene_render_targets.h"
#include "render_device.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"

namespace pathos {

	class VisualizeDepthVS : public ShaderStage {
	public:
		VisualizeDepthVS() : ShaderStage(GL_VERTEX_SHADER, "VisualizeDepthVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class VisualizeDepthFS : public ShaderStage {
	public:
		VisualizeDepthFS() : ShaderStage(GL_FRAGMENT_SHADER, "VisualizeDepthFS")
		{
			setFilepath("visualize_depth.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_VisualizeDepth, VisualizeDepthVS, VisualizeDepthFS);

}

namespace pathos {

	VisualizeDepth::VisualizeDepth()
		: dummyVAO(0)
	{
	}
	VisualizeDepth::~VisualizeDepth() {}

	void VisualizeDepth::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createVertexArrays(1, &dummyVAO);
	}

	void VisualizeDepth::destroyResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteVertexArrays(1, &dummyVAO);
	}

	void VisualizeDepth::render(RenderCommandList& cmdList, Scene* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(VisualizeDepth);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VisualizeDepth);

		cmdList.textureParameteri(sceneContext.sceneDepth, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
		cmdList.useProgram(program.getGLName());
		cmdList.disable(GL_DEPTH_TEST);

		cmdList.bindVertexArray(dummyVAO);
		cmdList.bindTextureUnit(0, sceneContext.sceneDepth);
		cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);

		cmdList.bindVertexArray(0);
		cmdList.bindTextureUnit(0, 0);
	}

}
