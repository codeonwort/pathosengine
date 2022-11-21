#include "volumetric_clouds.h"
#include "pathos/rhi/render_device.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/scene/volumetric_cloud_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/rhi/volume_texture.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/console.h"
// For NVidia STBN
#include "pathos/util/resource_finder.h"
#include "pathos/loader/imageloader.h"

#include "badger/math/minmax.h"

namespace pathos {

	static constexpr GLenum PF_volumetricCloud = GL_RGBA16F;

	static ConsoleVariable<float> cvar_cloud_resolution("r.cloud.resolution", 0.5f, "Resolution scale of cloud texture relative to screenSize");

	// #todo-cloud: Expose these cvars in VolumetricCloudComponent
	// But without a good GUI it's rather convenient to control them with cvars.
	static ConsoleVariable<float> cvar_cloud_earthRadius("r.cloud.earthRadius", (float)6.36e6, "Earth radius (in meters)");
	static ConsoleVariable<float> cvar_cloud_minY("r.cloud.minY", 1000.0f, "Cloud layer's min height from ground (in meters)");
	static ConsoleVariable<float> cvar_cloud_maxY("r.cloud.maxY", 6000.0f, "Cloud layer's max height from ground (in meters)");
	static ConsoleVariable<float> cvar_cloud_windSpeedX("r.cloud.windSpeedX", 0.002f, "Speed along u of the weather texture");
	static ConsoleVariable<float> cvar_cloud_windSpeedZ("r.cloud.windSpeedZ", 0.001f, "Speed along v of the weather texture");
	static ConsoleVariable<float> cvar_cloud_weatherScale("r.cloud.weatherScale", 0.01f, "Scale factor when sampling the weather texture");
	static ConsoleVariable<float> cvar_cloud_baseNoiseScale("r.cloud.baseNoiseScale", 0.1f, "Scale factor for base noise sampling");
	static ConsoleVariable<float> cvar_cloud_erosionNoiseScale("r.cloud.erosionNoiseScale", 0.2f, "Scale factor for erosion noise sampling");
	static ConsoleVariable<float> cvar_cloud_sunIntensityScale("r.cloud.sunIntensityScale", 200.0f, "Scale factor for Sun light's intensity");
	static ConsoleVariable<float> cvar_cloud_cloudCurliness("r.cloud.cloudCurliness", 0.1f, "Curliness of clouds");
	static ConsoleVariable<float> cvar_cloud_globalCoverage("r.cloud.globalCoverage", 0.0f, "Global cloud coverage");
	static ConsoleVariable<int32> cvar_cloud_minSteps("r.cloud.minSteps", 54, "Raymarch min steps");
	static ConsoleVariable<int32> cvar_cloud_maxSteps("r.cloud.maxSteps", 96, "Raymarch max steps");
	static ConsoleVariable<float> cvar_cloud_falloffDistance("r.cloud.falloffDistance", 100000.0f, "Clouds farther than this distance is invisible");

	// NOTE: If absorption or scattering coeff is too big (>= 0.1),
	//       cloud bottom layer will be too dark as sun light can't penetrate cloud density.
	static ConsoleVariable<float> cvar_cloud_sigma_a("r.cloud.sigma_a", 0.02f, "Absorption coefficient");
	static ConsoleVariable<float> cvar_cloud_sigma_s("r.cloud.sigma_s", 0.01f, "Scattering coefficient");

	// #todo-cloud: Temporal reprojection is incomplete.
	// Struggling with reprojecting first-hit positions to prev frame's UVs.
	static ConsoleVariable<int32> cvar_cloud_temporalReprojection("r.cloud.temporalReprojection", 0, "(WIP) Temporal reprojection");

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

		// w components are unused but not a big deal
		vector4 sunIntensity;
		vector4 sunDirection;

		float cloudCurliness;
		float globalCoverage;
		int32 minSteps;
		int32 maxSteps;

		float falloffDistance;
		float absorptionCoeff;
		float scatteringCoeff; // for both in-scattering and out-scattering
		float extinctionCoeff; // absorption + out-scattering

		int32 bTemporalReprojection;
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

		// #todo: Make STBN a system texture if needed.
		gRenderDevice->createTextures(GL_TEXTURE_3D, 1, &texSTBN);
		gRenderDevice->objectLabel(GL_TEXTURE, texSTBN, -1, "NVidiaSTBN");
		cmdList.textureStorage3D(texSTBN, 1, GL_RGBA8, 128, 128, 64);
		for (uint32 i = 0; i < 64; ++i) {
			char buf[256];
			sprintf_s(buf,
				"resources_external/NVidiaSpatioTemporalBlueNoise/STBN/%s_%u.png",
				"stbn_scalar_2Dx1Dx1D_128x128x64x1",
				i);
			std::string filepath = ResourceFinder::get().find(buf);
			CHECKF(filepath.size() > 0, "Run Setup.ps1 to download NVidia STBN");
			
			BitmapBlob* blob = pathos::loadImage(filepath.c_str());
			CHECK(blob->width == 128 && blob->height == 128 && blob->bpp == 32);
			cmdList.textureSubImage3D(texSTBN, 0, 0, 0, i,
				128, 128, 1, GL_RGBA, GL_UNSIGNED_BYTE, blob->getRawBytes());

			cmdList.textureParameteri(texSTBN, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(texSTBN, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			cmdList.textureParameteri(texSTBN, GL_TEXTURE_WRAP_S, GL_REPEAT);
			cmdList.textureParameteri(texSTBN, GL_TEXTURE_WRAP_T, GL_REPEAT);
			cmdList.textureParameteri(texSTBN, GL_TEXTURE_WRAP_R, GL_REPEAT);
		}
	}

	void VolumetricCloudPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteTextures(1, &texSTBN);
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
				vector3 intensity = scene->proxyList_directionalLight[0]->illuminance;
				intensity *= std::max(0.0f, cvar_cloud_sunIntensityScale.getFloat());
				uboData.sunIntensity = vector4(intensity, 0.0f);
				uboData.sunDirection = vector4(scene->proxyList_directionalLight[0]->wsDirection, 0.0f);
			} else {
				uboData.sunIntensity = vector4(0.0f);
				uboData.sunDirection = vector4(0.0f, -1.0f, 0.0f, 0.0f);
			}

			uboData.cloudCurliness = cvar_cloud_cloudCurliness.getFloat();
			uboData.globalCoverage = cvar_cloud_globalCoverage.getFloat();
			uboData.minSteps = std::max(cvar_cloud_minSteps.getInt(), 1);
			uboData.maxSteps = std::max(cvar_cloud_maxSteps.getInt(), uboData.minSteps);

			uboData.falloffDistance = std::max(1.0f, cvar_cloud_falloffDistance.getFloat());
			uboData.absorptionCoeff = badger::clamp(0.0f, cvar_cloud_sigma_a.getFloat(), 1.0f);
			uboData.scatteringCoeff = badger::clamp(0.0f, cvar_cloud_sigma_s.getFloat(), 1.0f);
			uboData.extinctionCoeff = std::min(1.0f, uboData.absorptionCoeff + uboData.scatteringCoeff);

			uboData.bTemporalReprojection = (0 != cvar_cloud_temporalReprojection.getInt());
			uboData.frameCounter = scene->frameNumber;
		}
		ubo.update(cmdList, UBO_VolumetricCloud::BINDING_POINT, &uboData);

		cmdList.bindTextureUnit(0, sceneContext.sceneDepth);
		cmdList.bindTextureUnit(1, scene->cloud->weatherTexture);
		cmdList.bindTextureUnit(2, scene->cloud->shapeNoise->getGLName());
		cmdList.bindTextureUnit(3, scene->cloud->erosionNoise->getGLName());
		cmdList.bindTextureUnit(4, texSTBN);
		cmdList.bindTextureUnit(5, sceneContext.getPrevVolumetricCloud(scene->frameNumber));
		cmdList.bindImageTexture(6, sceneContext.getVolumetricCloud(scene->frameNumber), 0, GL_FALSE, 0, GL_WRITE_ONLY, PF_volumetricCloud);

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
