#include "anti_aliasing_fxaa.h"
#include "pathos/shader/shader.h"
#include "pathos/render/scene_render_targets.h"

namespace pathos {

	void FXAA::initializeResources(RenderCommandList& cmdList)
	{
		std::string vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;

out vec2 uv;

void main() {
	const vec2[4] uvs = vec2[4](vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1));

	uv = uvs[gl_VertexID];
	gl_Position = vec4(position, 1.0);
}
)";

		Shader vs(GL_VERTEX_SHADER, "VS_FXAA");
		Shader fs(GL_FRAGMENT_SHADER, "FS_FXAA");

		vs.setSource(vshader);

		fs.addDefine("FXAA_PC 1");
		fs.addDefine("FXAA_GLSL_130 1");
		fs.addDefine("FXAA_GREEN_AS_LUMA 1");
		fs.addDefine("FXAA_QUALITY__PRESET 23");
		fs.loadSource("fxaa_fs.glsl");

		program = pathos::createProgram(vs, fs, "FXAA");

#define GET_UNIFORM(uniform) { uniform = cmdList.getUniformLocation(program, #uniform); }
		GET_UNIFORM(fxaaQualityRcpFrame        );
		GET_UNIFORM(fxaaConsoleRcpFrameOpt     );
		GET_UNIFORM(fxaaConsoleRcpFrameOpt2    );
		GET_UNIFORM(fxaaConsole360RcpFrameOpt2 );
		GET_UNIFORM(fxaaQualitySubpix          );
		GET_UNIFORM(fxaaQualityEdgeThreshold   );
		GET_UNIFORM(fxaaQualityEdgeThresholdMin);
		GET_UNIFORM(fxaaConsoleEdgeSharpness   );
		GET_UNIFORM(fxaaConsoleEdgeThreshold   );
		GET_UNIFORM(fxaaConsoleEdgeThresholdMin);
		GET_UNIFORM(fxaaConsole360ConstDir     );
#undef GET_UNIFORM

		//////////////////////////////////////////////////////////////////////////

		cmdList.createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		//checkFramebufferStatus(cmdList, fbo); // #todo-framebuffer: Can't check completeness now
	}

	void FXAA::releaseResources(RenderCommandList& cmdList)
	{
		cmdList.deleteProgram(program);
		cmdList.deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void FXAA::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(FXAA);

		const GLuint input0 = getInput(EPostProcessInput::PPI_0); // toneMappingResult
		const GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // sceneFinal or backbuffer

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		// #note-fxaa: See the FXAA pixel shader for details
		float sharpness = 0.5f;
		float subpix = 0.75f;
		float edge_threshold = 0.166f;
		float edge_threshold_min = 0.0f; // 0.0833f;
		float console_edge_sharpness = 8.0f;
		float console_edge_threshold = 0.125f;
		float console_edge_threshold_min = 0.05f;

		glm::vec2 inv_size(1.0f / (float)sceneContext.sceneWidth, 1.0f / (float)sceneContext.sceneHeight);
		glm::vec4 inv_size_4(-inv_size.x, -inv_size.y, inv_size.x, inv_size.y);
		glm::vec4 sharp_param = sharpness * inv_size_4;
		glm::vec4 sharp2_param = 2.0f * inv_size_4;
		glm::vec4 sharp3_param = glm::vec4(8.0f, 8.0f, -4.0f, -4.0f) * inv_size_4;

		cmdList.useProgram(program);

		cmdList.uniform2f(fxaaQualityRcpFrame        , inv_size.x, inv_size.y);
		cmdList.uniform4f(fxaaConsoleRcpFrameOpt	 , sharp_param.x, sharp_param.y, sharp_param.z, sharp_param.w);
		cmdList.uniform4f(fxaaConsoleRcpFrameOpt2	 , sharp2_param.x, sharp2_param.y, sharp2_param.z, sharp2_param.w);
		cmdList.uniform4f(fxaaConsole360RcpFrameOpt2 , sharp3_param.x, sharp3_param.y, sharp3_param.z, sharp3_param.w);
		cmdList.uniform1f(fxaaQualitySubpix			 , subpix);
		cmdList.uniform1f(fxaaQualityEdgeThreshold	 , edge_threshold);
		cmdList.uniform1f(fxaaQualityEdgeThresholdMin, edge_threshold_min);
		cmdList.uniform1f(fxaaConsoleEdgeSharpness	 , console_edge_sharpness);
		cmdList.uniform1f(fxaaConsoleEdgeThreshold	 , console_edge_threshold);
		cmdList.uniform1f(fxaaConsoleEdgeThresholdMin, console_edge_threshold_min);
		cmdList.uniform4f(fxaaConsole360ConstDir	 , 1.0f, -1.0f, 0.25f, -0.25f);

		if (output0 == 0) {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
		} else {
			cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);
		}

		cmdList.textureParameteri(input0, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(input0, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.bindTextureUnit(0, input0);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
	}

}
