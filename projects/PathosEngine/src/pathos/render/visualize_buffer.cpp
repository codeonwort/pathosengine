#include "visualize_buffer.h"
#include "scene_render_targets.h"
#include "render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"

namespace pathos {

	class VisualizeBufferVS : public ShaderStage {
	public:
		VisualizeBufferVS() : ShaderStage(GL_VERTEX_SHADER, "VisualizeBufferVS")
		{
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class VisualizeBufferFS : public ShaderStage {
	public:
		VisualizeBufferFS() : ShaderStage(GL_FRAGMENT_SHADER, "VisualizeBufferFS")
		{
			setFilepath("visualize_buffer.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_VisualizeBuffer, VisualizeBufferVS, VisualizeBufferFS);

}

namespace pathos {

	VisualizeBufferPass::VisualizeBufferPass()
		: dummyVAO(0)
	{
	}
	VisualizeBufferPass::~VisualizeBufferPass() {}

	void VisualizeBufferPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createVertexArrays(1, &dummyVAO);
	}

	void VisualizeBufferPass::destroyResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteVertexArrays(1, &dummyVAO);
	}

	void VisualizeBufferPass::render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera)
	{
		SCOPED_DRAW_EVENT(VisualizeBufferPass);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VisualizeBuffer);

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
