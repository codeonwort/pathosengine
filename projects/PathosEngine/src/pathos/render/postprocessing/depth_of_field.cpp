#include "depth_of_field.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"

namespace pathos {

	static ConsoleVariable<float> cvar_focal_distance("r.dof.focal_distance", 50.0f, "focal distance of DoF");
	static ConsoleVariable<float> cvar_focal_depth("r.dof.focal_depth", 1000.0f, "focal depth of DoF");
	static ConsoleVariable<float> cvar_max_radius("r.dof.max_radius", 4.5f, "max radius of DoF kernel");

	struct UBO_DoF {
		float focalDistance;
		float focalDepth;
		float maxRadius;
	};

	DepthOfField::~DepthOfField() {
		markDestroyed();
	}

	void DepthOfField::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createVertexArrays(1, &vao);

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		//checkFramebufferStatus(cmdList, fbo); // #todo-framebuffer: Can't check completeness now

		// compute program. output is transposed.
		program_subsum2D = createSubsumShader();

		// program with vertex and fragment shaders.
		program_blur = createBlurShader();
		uboBlur.init<UBO_DoF>();
	}

	void DepthOfField::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteVertexArrays(1, &vao);
		gRenderDevice->deleteFramebuffers(1, &fbo);
		gRenderDevice->deleteProgram(program_subsum2D);
		gRenderDevice->deleteProgram(program_blur);

		markDestroyed();
	}

	void DepthOfField::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(DepthOfField);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneFinal
		GLuint output0 = getOutput(EPostProcessOutput::PPO_0); // backbuffer

		{
			SCOPED_DRAW_EVENT(Subsum);

			cmdList.useProgram(program_subsum2D);
			cmdList.bindImageTexture(0, input0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			cmdList.bindImageTexture(1, sceneContext.dofSubsum0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			cmdList.dispatchCompute(sceneContext.sceneHeight, 1, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			cmdList.bindImageTexture(0, sceneContext.dofSubsum0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			cmdList.bindImageTexture(1, sceneContext.dofSubsum1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			cmdList.dispatchCompute(sceneContext.sceneWidth, 1, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		/* sceneContext.dofSubsum1 now holds subsum table */
		
		{
			SCOPED_DRAW_EVENT(Blur);

			// apply box blur whose strength is relative to the difference between pixel depth and focal depth
			cmdList.useProgram(program_blur);
			cmdList.bindTextureUnit(0, sceneContext.dofSubsum1);

			UBO_DoF uboData;
			uboData.focalDistance = cvar_focal_distance.getFloat();
			uboData.focalDepth = cvar_focal_depth.getFloat();
			uboData.maxRadius = cvar_max_radius.getFloat();
			uboBlur.update(cmdList, 1, &uboData);

			if (output0 == 0) {
				cmdList.bindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			else {
				cmdList.bindFramebuffer(GL_FRAMEBUFFER, fbo);
				// #todo-misc: I sometimes make a typo like cmdList.namedFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, output0, 0);
				// and do not realize what I did wrong. Need a utility for validation.
				cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);
			}

			cmdList.bindVertexArray(vao);
			cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
			cmdList.bindVertexArray(0);
		}
	}

	GLuint DepthOfField::createSubsumShader() {
		Shader cs(GL_COMPUTE_SHADER, "CS_DoF_Subsum");
		cs.loadSource("subsum.glsl");

		GLuint program = pathos::createProgram(cs, "DoF_Subsum");
		CHECK(program);

		return program;
	}

	GLuint DepthOfField::createBlurShader() {
		Shader vs(GL_VERTEX_SHADER, "VS_DoF_Blur");
		Shader fs(GL_FRAGMENT_SHADER, "FS_DoF_Blur");
		vs.loadSource("fullscreen_quad.glsl");
		fs.loadSource("depth_of_field.glsl");

		GLuint program = pathos::createProgram(vs, fs, "DoF_Blur");
		CHECK(program);

		return program;
	}

}
