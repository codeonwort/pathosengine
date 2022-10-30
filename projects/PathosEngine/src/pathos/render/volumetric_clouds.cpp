#include "volumetric_clouds.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/scene/volumetric_cloud_component.h"
#include "pathos/light/directional_light_component.h"
#include "pathos/texture/volume_texture.h"
#include "pathos/shader/shader_program.h"
#include "pathos/console.h"

namespace pathos {

	static constexpr GLenum PF_volumetricCloud = GL_RGBA16F;

	static ConsoleVariable<float> cvar_cloud_resolution("r.cloud.resolution", 0.5f, "Resolution scale of cloud texture relative to screenSize");

	// #todo-cloud: Expose these cvars in VolumetricCloudComponent
	// But without a good GUI it's rather convenient to control them with cvars.
	static ConsoleVariable<float> cvar_cloud_earthRadius("r.cloud.earthRadius", (float)6.36e6, "Earth radius");
	static ConsoleVariable<float> cvar_cloud_minY("r.cloud.minY", 2000.0f, "Cloud layer range (min)");
	static ConsoleVariable<float> cvar_cloud_maxY("r.cloud.maxY", 5000.0f, "Cloud layer range (max)");
	static ConsoleVariable<float> cvar_cloud_windSpeedX("r.cloud.windSpeedX", 0.002f, "Speed along u of the weather texture");
	static ConsoleVariable<float> cvar_cloud_windSpeedZ("r.cloud.windSpeedZ", 0.001f, "Speed along v of the weather texture");
	static ConsoleVariable<float> cvar_cloud_weatherScale("r.cloud.weatherScale", 0.01f, "Scale factor when sampling the weather texture");
	static ConsoleVariable<float> cvar_cloud_baseNoiseScale("r.cloud.baseNoiseScale", 0.005f, "Scale factor for base noise sampling");
	static ConsoleVariable<float> cvar_cloud_erosionNoiseScale("r.cloud.erosionNoiseScale", 0.25f, "Scale factor for erosion noise sampling");
	static ConsoleVariable<float> cvar_cloud_cloudCurliness("r.cloud.cloudCurliness", 0.1f, "Curliness of clouds");
	
	// #todo-cloud: Deprecated.
	static ConsoleVariable<float> cvar_cloud_coverageOffset("r.cloud.coverageOffset", 0.0f, "Cloud coverage offset");
	static ConsoleVariable<float> cvar_cloud_baseNoiseOffset("r.cloud.baseNoiseOffset", 0.0f, "Base noise offset");

	struct UBO_VolumetricCloud {
		static constexpr uint32 BINDING_POINT = 1;

		float earthRadius;
		float cloudLayerMinY;
		float cloudLayerMaxY;
		float windSpeedX;

		float windSpeedZ;
		float weatherScale;
		float baseNoiseScale;
		float erosionNoiseScale;

		vector4 sunIntensity;

		float cloudCurliness;
		float cloudCoverageOffset;
		float baseNoiseOffset;
		uint32 frameCounter;
	};

	// #todo-cloud: Use clearTexImage instead of dispatching a CS.
	class VolumetricCloudClearCS : public ShaderStage {
	public:
		VolumetricCloudClearCS() : ShaderStage(GL_COMPUTE_SHADER, "VolumetricCloudClearCS") {
			setFilepath("volumetric_clouds_clear.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_VolumetricCloudClear, VolumetricCloudClearCS);

	class VolumetricCloudCS : public ShaderStage {
	public:
		VolumetricCloudCS() : ShaderStage(GL_COMPUTE_SHADER, "VolumetricCloudCS") {
			setFilepath("volumetric_clouds.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_VolumetricCloud, VolumetricCloudCS);

}

namespace pathos {

	void VolumetricCloudPass::initializeResources(RenderCommandList& cmdList) {
		ubo.init<UBO_VolumetricCloud>();
	}

	void VolumetricCloudPass::releaseResources(RenderCommandList& cmdList) {
		//
	}

	void VolumetricCloudPass::renderVolumetricCloud(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(VolumetricCloud);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VolumetricCloud);

		const uint32 sceneWidth = sceneContext.sceneWidth;
		const uint32 sceneHeight = sceneContext.sceneHeight;

		float resolutionScale = glm::clamp(cvar_cloud_resolution.getFloat(), 0.1f, 1.0f);
		recreateRenderTarget(cmdList, sceneWidth, sceneHeight, resolutionScale);

		const bool bRenderClouds = scene->isVolumetricCloudValid();
		if (!bRenderClouds) {
#if 0
			// #todo-cloud: How to do this correctly?
			// (GL_RGBA, GL_HALF_FLOAT) for (PF_volumetricCloud = GL_RGBA16F)
			GLhalf clearValues[] = { 0, 0, 0, 1 }; // ???
			cmdList.clearTexImage(sceneContext.getVolumetricCloud(scene->frameNumber), 0, GL_RGBA, GL_HALF_FLOAT, clearValues);
#else
			ShaderProgram& clearProgram = FIND_SHADER_PROGRAM(Program_VolumetricCloudClear);
			cmdList.useProgram(clearProgram.getGLName());
			GLuint workGroupsX = (GLuint)ceilf((float)(resolutionScale * sceneWidth) / 16.0f);
			GLuint workGroupsY = (GLuint)ceilf((float)(resolutionScale * sceneHeight) / 16.0f);
			cmdList.bindImageTexture(0, sceneContext.getVolumetricCloud(scene->frameNumber), 0, GL_FALSE, 0, GL_WRITE_ONLY, PF_volumetricCloud);
			cmdList.dispatchCompute(workGroupsX, workGroupsY, 1);
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
#endif
			return;
		}

		cmdList.textureParameteri(sceneContext.sceneDepth, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

		cmdList.useProgram(program.getGLName());

		UBO_VolumetricCloud uboData;
		{
			uboData.earthRadius = cvar_cloud_earthRadius.getFloat();
			uboData.cloudLayerMinY = cvar_cloud_minY.getFloat();
			uboData.cloudLayerMaxY = cvar_cloud_maxY.getFloat();
			uboData.windSpeedX = cvar_cloud_windSpeedX.getFloat();

			uboData.windSpeedZ = cvar_cloud_windSpeedZ.getFloat();
			uboData.weatherScale = cvar_cloud_weatherScale.getFloat();
			uboData.baseNoiseScale = cvar_cloud_baseNoiseScale.getFloat();
			uboData.erosionNoiseScale = cvar_cloud_erosionNoiseScale.getFloat();

			if (scene->proxyList_directionalLight.size() > 0) {
				uboData.sunIntensity = vector4(scene->proxyList_directionalLight[0]->radiance, 0.0f);
			} else {
				uboData.sunIntensity = vector4(0.0f);
			}

			uboData.cloudCurliness = cvar_cloud_cloudCurliness.getFloat();
			uboData.cloudCoverageOffset = cvar_cloud_coverageOffset.getFloat();
			uboData.baseNoiseOffset = cvar_cloud_baseNoiseOffset.getFloat();
			uboData.frameCounter = scene->frameNumber;
		}
		ubo.update(cmdList, UBO_VolumetricCloud::BINDING_POINT, &uboData);

		cmdList.bindTextureUnit(0, sceneContext.sceneDepth);
		cmdList.bindTextureUnit(1, scene->cloud->weatherTexture);
		cmdList.bindTextureUnit(2, scene->cloud->shapeNoise->getGLName());
		cmdList.bindTextureUnit(3, scene->cloud->erosionNoise->getGLName());
		cmdList.bindTextureUnit(4, sceneContext.getPrevVolumetricCloud(scene->frameNumber));
		cmdList.bindImageTexture(5, sceneContext.getVolumetricCloud(scene->frameNumber), 0, GL_FALSE, 0, GL_WRITE_ONLY, PF_volumetricCloud);

		GLuint workGroupsX = (GLuint)ceilf((float)(resolutionScale * sceneWidth) / 16.0f);
		GLuint workGroupsY = (GLuint)ceilf((float)(resolutionScale * sceneHeight) / 16.0f);
		cmdList.dispatchCompute(workGroupsX, workGroupsY, 1);

		cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void VolumetricCloudPass::recreateRenderTarget(RenderCommandList& cmdList, uint32 inWidth, uint32 inHeight, float inResolutionScale)
	{
		CHECKF(inWidth != 0 && inHeight != 0, "Invalid size for cloud render target");

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const uint32 targetWidth = (uint32)(inWidth * inResolutionScale);
		const uint32 targetHeight = (uint32)(inHeight * inResolutionScale);

		CHECK(targetWidth != 0 && targetHeight != 0);

		if (renderTargetWidth != targetWidth || renderTargetHeight != targetHeight) {
			if (sceneContext.volumetricCloudA != 0) {
				gRenderDevice->deleteTextures(1, &sceneContext.volumetricCloudA);
				sceneContext.volumetricCloudA = 0;
			}
			if (sceneContext.volumetricCloudB != 0) {
				gRenderDevice->deleteTextures(1, &sceneContext.volumetricCloudB);
				sceneContext.volumetricCloudB = 0;
			}

			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &sceneContext.volumetricCloudA);
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &sceneContext.volumetricCloudB);
			gRenderDevice->objectLabel(GL_TEXTURE, sceneContext.volumetricCloudA, -1, "Texture_CloudRT0");
			gRenderDevice->objectLabel(GL_TEXTURE, sceneContext.volumetricCloudB, -1, "Texture_CloudRT1");

			renderTargetWidth = targetWidth;
			renderTargetHeight = targetHeight;

			cmdList.textureStorage2D(sceneContext.volumetricCloudA, 1, PF_volumetricCloud, renderTargetWidth, renderTargetHeight);
			cmdList.textureStorage2D(sceneContext.volumetricCloudB, 1, PF_volumetricCloud, renderTargetWidth, renderTargetHeight);
		}
	}

}