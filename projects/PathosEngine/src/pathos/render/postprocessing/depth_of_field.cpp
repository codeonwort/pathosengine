#include "depth_of_field.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"
#include "pathos/shader/shader_program.h"

namespace pathos {
	
	// Output is transposed
	class DOF_PrefixSum : public ShaderStage {
	public:
		DOF_PrefixSum() : ShaderStage(GL_COMPUTE_SHADER, "DOF_PrefixSum") {
			setFilepath("prefix_sum.glsl");
		}
	};

	class DOF_BlurVS : public ShaderStage {
	public:
		DOF_BlurVS() : ShaderStage(GL_VERTEX_SHADER, "DOF_BlurVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class DOF_BlurFS : public ShaderStage {
	public:
		DOF_BlurFS() : ShaderStage(GL_FRAGMENT_SHADER, "DOF_BlurFS") {
			setFilepath("depth_of_field.glsl");
		}
	};

	DEFINE_COMPUTE_PROGRAM(Program_DOF_PrefixSum, DOF_PrefixSum);
	DEFINE_SHADER_PROGRAM2(Program_DOF_Blur, DOF_BlurVS, DOF_BlurFS);

}

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
		
		uboBlur.init<UBO_DoF>();
	}

	void DepthOfField::releaseResources(RenderCommandList& cmdList)
	{
		gRenderDevice->deleteVertexArrays(1, &vao);
		gRenderDevice->deleteFramebuffers(1, &fbo);

		markDestroyed();
	}

	void DepthOfField::renderPostProcess(RenderCommandList& cmdList, PlaneGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(DepthOfField);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint input0 = getInput(EPostProcessInput::PPI_0); // sceneFinal
		GLuint output0 = getOutput(EPostProcessOutput::PPO_0);

		ShaderProgram& program_prefix_sum = FIND_SHADER_PROGRAM(Program_DOF_PrefixSum);
		ShaderProgram& program_blur = FIND_SHADER_PROGRAM(Program_DOF_Blur);

		{
			SCOPED_DRAW_EVENT(DepthOfField_Subsum);

			cmdList.useProgram(program_prefix_sum.getGLName());
			cmdList.bindImageTexture(0, input0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			cmdList.bindImageTexture(1, sceneContext.dofSubsum0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			cmdList.dispatchCompute(sceneContext.sceneHeight, 1, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			cmdList.bindImageTexture(0, sceneContext.dofSubsum0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			cmdList.bindImageTexture(1, sceneContext.dofSubsum1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			cmdList.dispatchCompute(sceneContext.sceneWidth, 1, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		/* sceneContext.dofSubsum1 now holds prefix sum table */
		
		{
			SCOPED_DRAW_EVENT(DepthOfField_Blur);

			// apply box blur whose strength is relative to the difference between pixel depth and focal depth
			cmdList.useProgram(program_blur.getGLName());
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

	bool DepthOfField::isAvailable() const {
		return cvar_focal_distance.getFloat() > 0.0f
			&& cvar_focal_depth.getFloat() > 0.0f
			&& cvar_max_radius.getFloat() > 0.0f;
	}

}
