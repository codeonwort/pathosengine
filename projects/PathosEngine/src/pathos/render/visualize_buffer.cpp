#include "visualize_buffer.h"
#include "scene_render_targets.h"
#include "render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"

namespace pathos {

	struct UBO_VisualizeBuffer {
		int32 viewmode;
	};

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

	static ConsoleVariable<int32> cvar_viewmode("r.viewmode", 0,
		"0 = disable visualization, 1 = sceneDepth, 2 = albedo, 3 = worldNormal,\
		 4 = metallic, 5 = roughness, 6 = emissive, 7 = ssao");

	VisualizeBufferPass::VisualizeBufferPass()
		: dummyVAO(0)
	{
	}
	VisualizeBufferPass::~VisualizeBufferPass() {}

	void VisualizeBufferPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createVertexArrays(1, &dummyVAO);
		ubo.init<UBO_VisualizeBuffer>();
	}

	void VisualizeBufferPass::destroyResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteVertexArrays(1, &dummyVAO);
	}

	void VisualizeBufferPass::render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera)
	{
		if (cvar_viewmode.getInt() == 0) {
			return;
		}

		SCOPED_DRAW_EVENT(VisualizeBufferPass);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VisualizeBuffer);

		UBO_VisualizeBuffer uboData;
		uboData.viewmode = cvar_viewmode.getInt();
		ubo.update(cmdList, 1, &uboData);

		cmdList.textureParameteri(sceneContext.sceneDepth, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
		cmdList.useProgram(program.getGLName());
		cmdList.disable(GL_DEPTH_TEST);

		cmdList.bindVertexArray(dummyVAO);

		// Bind buffers as SRVs
		cmdList.bindTextureUnit(0, sceneContext.sceneDepth);
		cmdList.bindTextureUnit(1, sceneContext.gbufferA);
		cmdList.bindTextureUnit(2, sceneContext.gbufferB);
		cmdList.bindTextureUnit(3, sceneContext.gbufferC);
		cmdList.bindTextureUnit(4, sceneContext.ssaoMap);

		cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);

		cmdList.bindVertexArray(0);
		cmdList.bindTextureUnit(0, 0);
	}

}