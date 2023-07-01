#include "anti_aliasing_fxaa.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/util/engine_util.h"

namespace pathos {

	struct UBO_FXAA {
		static constexpr uint32 BINDING_SLOT = 1;

		vector4 fxaaConsoleRcpFrameOpt;
		vector4 fxaaConsoleRcpFrameOpt2;
		vector4 fxaaConsole360RcpFrameOpt2;
		vector4 fxaaConsole360ConstDir;
		vector2 fxaaQualityRcpFrame;
		float fxaaQualitySubpix;
		float fxaaQualityEdgeThreshold;
		float fxaaQualityEdgeThresholdMin;
		float fxaaConsoleEdgeSharpness;
		float fxaaConsoleEdgeThreshold;
		float fxaaConsoleEdgeThresholdMin;
	};

	class FXAAPassVS : public ShaderStage {
	public:
		FXAAPassVS() : ShaderStage(GL_VERTEX_SHADER, "FXAAPassVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class FXAAPassFS : public ShaderStage {
	public:
		FXAAPassFS() : ShaderStage(GL_FRAGMENT_SHADER, "FXAAPassFS") {
			addDefine("FXAA_PC", 1);
			addDefine("FXAA_GLSL_130", 1);
			addDefine("FXAA_GREEN_AS_LUMA", 1);
			addDefine("FXAA_QUALITY__PRESET", 23);
			setFilepath("fxaa_fs.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_FXAA, FXAAPassVS, FXAAPassFS);

}

namespace pathos {

	void FXAA::initializeResources(RenderCommandList& cmdList) {
		ubo.init<UBO_FXAA>("UBO_FXAA");

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_FXAA");
	}

	void FXAA::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void FXAA::renderPostProcess(RenderCommandList& cmdList, MeshGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(FXAA);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneColorToneMapped
		const GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // sceneColorAA or sceneFinal
		CHECKF(output0 != 0, "Post processes do not write to the backbuffer anymore");

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		// #note-fxaa: See the FXAA pixel shader for details
		float sharpness = 0.5f;
		float subpix = 0.75f;
		float edge_threshold = 0.166f;
		float edge_threshold_min = 0.0f; // 0.0833f;
		float console_edge_sharpness = 8.0f;
		float console_edge_threshold = 0.125f;
		float console_edge_threshold_min = 0.05f;

		vector2 inv_size(1.0f / (float)sceneContext.sceneWidth, 1.0f / (float)sceneContext.sceneHeight);
		vector4 inv_size_4(-inv_size.x, -inv_size.y, inv_size.x, inv_size.y);
		vector4 sharp_param = sharpness * inv_size_4;
		vector4 sharp2_param = 2.0f * inv_size_4;
		vector4 sharp3_param = vector4(8.0f, 8.0f, -4.0f, -4.0f) * inv_size_4;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_FXAA);
		cmdList.useProgram(program.getGLName());

		UBO_FXAA uboData;
		uboData.fxaaQualityRcpFrame          = vector2(inv_size.x, inv_size.y);
		uboData.fxaaConsoleRcpFrameOpt	     = vector4(sharp_param.x, sharp_param.y, sharp_param.z, sharp_param.w);
		uboData.fxaaConsoleRcpFrameOpt2	     = vector4(sharp2_param.x, sharp2_param.y, sharp2_param.z, sharp2_param.w);
		uboData.fxaaConsole360RcpFrameOpt2   = vector4(sharp3_param.x, sharp3_param.y, sharp3_param.z, sharp3_param.w);
		uboData.fxaaQualitySubpix			 = subpix;
		uboData.fxaaQualityEdgeThreshold	 = edge_threshold;
		uboData.fxaaQualityEdgeThresholdMin  = edge_threshold_min;
		uboData.fxaaConsoleEdgeSharpness	 = console_edge_sharpness;
		uboData.fxaaConsoleEdgeThreshold	 = console_edge_threshold;
		uboData.fxaaConsoleEdgeThresholdMin  = console_edge_threshold_min;
		uboData.fxaaConsole360ConstDir	     = vector4(1.0f, -1.0f, 0.25f, -0.25f);
		ubo.update(cmdList, UBO_FXAA::BINDING_SLOT, &uboData);

		cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);
		pathos::checkFramebufferStatus(cmdList, fbo, "fxaa");

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		cmdList.textureParameteri(input0, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(input0, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.bindTextureUnit(0, input0);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
	}

}
