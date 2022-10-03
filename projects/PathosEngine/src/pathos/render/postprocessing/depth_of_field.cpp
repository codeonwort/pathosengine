#include "depth_of_field.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"
#include "pathos/shader/shader_program.h"
#include "pathos/util/engine_util.h"

namespace pathos {

	struct UBO_PrefixSum {
		int32 fetchOffset;
		int32 maxImageLength;
	};
	
	// Output is transposed
	class DOF_PrefixSum : public ShaderStage {
	public:
		DOF_PrefixSum() : ShaderStage(GL_COMPUTE_SHADER, "DOF_PrefixSum") {
			setFilepath("prefix_sum.glsl");

			int32 maxBucketSize = gRenderDevice->getCapabilities().glMaxComputeWorkGroupSize[0];
			addDefine("BUCKET_SIZE", maxBucketSize);
		}
	};

	struct UBO_DoF {
		float focalDistance;
		float focalDepth;
		float maxRadius;
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

	static ConsoleVariable<float> cvar_focal_distance("r.dof.focal_distance", 200.0f, "focal distance of DoF");
	static ConsoleVariable<float> cvar_focal_depth("r.dof.focal_depth", 10000.0f, "focal depth of DoF");
	static ConsoleVariable<float> cvar_max_radius("r.dof.max_radius", 4.5f, "max radius of DoF kernel");

	DepthOfField::~DepthOfField() {
		markDestroyed();
	}

	void DepthOfField::initializeResources(RenderCommandList& cmdList)
	{
		gRenderDevice->createVertexArrays(1, &vao);

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		
		uboPrefixSum.init<UBO_PrefixSum>();
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

		constexpr GLenum PF_dofSubsum = GL_RGBA32F;

		{
			SCOPED_DRAW_EVENT(DepthOfField_Subsum);

			// Actually we can process a double of workGroupSizeX on one dispatch,
			// so bucketSize here is twice of BUCKET_SIZE.
			const int32 bucketSize = 2 * gRenderDevice->getCapabilities().glMaxComputeWorkGroupSize[0];
			cmdList.useProgram(program_prefix_sum.getGLName());

			// CAUTION: DoF executes after super resolution.
			const uint32 SCENE_WIDTH = sceneContext.sceneWidthSuperRes;
			const uint32 SCENE_HEIGHT = sceneContext.sceneHeightSuperRes;

			// Prefix sum shader can process only 2048 columns at once, so we split up the work into buckets.
			{
				const int32 numRuns = (int32)(::ceilf((float)SCENE_WIDTH / bucketSize));
				UBO_PrefixSum uboData;
				uboData.fetchOffset = 0;
				uboData.maxImageLength = SCENE_WIDTH;
				
				for (int32 i = 0; i < numRuns; ++i) {
					uboPrefixSum.update(cmdList, 1, &uboData);

					cmdList.bindImageTexture(0, input0, 0, GL_FALSE, 0, GL_READ_ONLY, PF_dofSubsum);
					cmdList.bindImageTexture(1, sceneContext.dofSubsum0, 0, GL_FALSE, 0, GL_READ_WRITE, PF_dofSubsum);
					cmdList.dispatchCompute(SCENE_HEIGHT, 1, 1);
					cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

					uboData.fetchOffset += bucketSize;
				}
			}
			{
				const int32 numRuns = (int32)(::ceilf((float)SCENE_HEIGHT / bucketSize));
				UBO_PrefixSum uboData;
				uboData.fetchOffset = 0;
				uboData.maxImageLength = SCENE_HEIGHT;

				for (int32 i = 0; i < numRuns; ++i) {
					uboPrefixSum.update(cmdList, 1, &uboData);

					cmdList.bindImageTexture(0, sceneContext.dofSubsum0, 0, GL_FALSE, 0, GL_READ_ONLY, PF_dofSubsum);
					cmdList.bindImageTexture(1, sceneContext.dofSubsum1, 0, GL_FALSE, 0, GL_READ_WRITE, PF_dofSubsum);
					cmdList.dispatchCompute(SCENE_WIDTH, 1, 1);
					cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

					uboData.fetchOffset += bucketSize;
				}
			}
		}

		/* sceneContext.dofSubsum1 now holds prefix sum table */
		
		{
			SCOPED_DRAW_EVENT(DepthOfField_Blur);

			// apply box blur whose strength is relative to the difference between pixel depth and focal depth
			cmdList.useProgram(program_blur.getGLName());
			cmdList.bindTextureUnit(0, sceneContext.dofSubsum1);
			cmdList.bindTextureUnit(1, sceneContext.gbufferA);
			cmdList.bindTextureUnit(2, sceneContext.gbufferB);
			cmdList.bindTextureUnit(3, sceneContext.gbufferC);

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
				cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, output0, 0);
				pathos::checkFramebufferStatus(cmdList, fbo, "depth_of_field");
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
