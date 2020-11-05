#include "sky_clouds.h"
#include "render_device.h"
#include "scene_render_targets.h"
#include "pathos/thread/engine_thread.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/texture/volume_texture.h"

namespace pathos {

	class VolumetricCloudCS : public ShaderStage {
	public:
		VolumetricCloudCS() : ShaderStage(GL_COMPUTE_SHADER, "VolumetricCloudCS") {
			setFilepath("volumetric_clouds.glsl");
		}
	};

	DEFINE_COMPUTE_PROGRAM(Program_VolumetricCloud, VolumetricCloudCS);

}

namespace pathos {

	void VolumetricCloudActor::setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise)
	{
		weatherTexture = inWeatherTexture;
		shapeNoise = inShapeNoise;
		erosionNoise = inErosionNoise;
	}

	bool VolumetricCloudActor::hasValidResources() const
	{
		return weatherTexture != 0
			&& shapeNoise != nullptr && shapeNoise->isValid()
			&& erosionNoise != nullptr && erosionNoise->isValid();
	}

}

namespace pathos {

	VolumetricCloud::~VolumetricCloud()
	{
		//if (renderTarget != 0) {
		//	gRenderDevice->deleteTextures(1, &renderTarget);
		//}
	}

	void VolumetricCloud::initializeResources(RenderCommandList& cmdList)
	{
		//
	}

	void VolumetricCloud::destroyResources(RenderCommandList& cmdList)
	{
		//
	}

	void VolumetricCloud::render(RenderCommandList& cmdList, const VolumetricCloudSettings& settings)
	{
		SCOPED_DRAW_EVENT(VolumetricCloud);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		recreateRenderTarget(cmdList, settings.renderTargetWidth, settings.renderTargetHeight);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VolumetricCloud);
		GLuint workGroupsX = (GLuint)ceilf((float)(settings.renderTargetWidth) / 16.0f);
		GLuint workGroupsY = (GLuint)ceilf((float)(settings.renderTargetHeight) / 16.0f);

		cmdList.useProgram(program.getGLName());
		//cmdList.bindImageTexture(0, settings.weatherTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
		cmdList.bindTextureUnit(0, settings.weatherTexture);
		cmdList.bindTextureUnit(1, settings.shapeNoiseTexture);
		cmdList.bindTextureUnit(2, settings.erosionNoiseTexture);
		cmdList.bindImageTexture(3, sceneContext.volumetricCloud, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		cmdList.dispatchCompute(workGroupsX, workGroupsY, 1);
		cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void VolumetricCloud::recreateRenderTarget(RenderCommandList& cmdList, uint32 inWidth, uint32 inHeight)
	{
		CHECKF(inWidth != 0 && inHeight != 0, "Invalid size for cloud render target");

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		if (renderTargetWidth != inWidth || renderTargetHeight != inHeight) {
			if (sceneContext.volumetricCloud != 0) {
				gRenderDevice->deleteTextures(1, &sceneContext.volumetricCloud);
				sceneContext.volumetricCloud = 0;
			}

			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &sceneContext.volumetricCloud);
			glObjectLabel(GL_TEXTURE, sceneContext.volumetricCloud, -1, "Texture_CloudRT");

			renderTargetWidth = inWidth;
			renderTargetHeight = inHeight;

			cmdList.textureStorage2D(sceneContext.volumetricCloud, 1, GL_RGBA16F, renderTargetWidth, renderTargetHeight);
		}
	}

}
