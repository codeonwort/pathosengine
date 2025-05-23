#include "volumetric_clouds.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/sampler.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/mesh/geometry.h"
#include "pathos/scene/volumetric_cloud_component.h"
#include "pathos/scene/directional_light_component.h"
#include "pathos/engine_policy.h"
#include "pathos/console.h"

// For NVidia STBN
#include "pathos/loader/image_loader.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#include "badger/math/minmax.h"

namespace pathos {

	static constexpr GLenum PF_volumetricCloud = GL_RGBA16F;

	static ConsoleVariable<int32> cvar_enable_volClouds("r.cloud.enable", 1, "[0/1] Toggle volumetric clouds");
	static ConsoleVariable<float> cvar_cloud_resolution("r.cloud.resolution", 0.5f, "Resolution scale of cloud texture relative to screenSize (no effect if panorama mode)");

	static ConsoleVariable<int32> cvar_cloud_panorama("r.cloud.panorama", 0, "[0/1] Toggle volumetric clouds panorama mode");
	static ConsoleVariable<int32> cvar_cloud_panoramaWidth("r.cloud.panoramaWidth", 2048, "Panorama texture width");
	static ConsoleVariable<int32> cvar_cloud_panoramaHeight("r.cloud.panoramaHeight", 1024, "Panorama texture height");
	static ConsoleVariable<float> cvar_cloud_panoramaCameraY("r.cloud.panoramaCameraY", 0.0f, "Panorama mode uses fixed camera altitude");

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
		float panoramaCameraY;
	};

	struct UBO_VolumetricCloudPost {
		static constexpr uint32 BINDING_POINT = 1;

		uint32 bPanorama;
		float  zFarPlane;
		uint32 _pad1;
		uint32 _pad2;
	};

	template<bool bPanorama>
	class VolumetricCloudCS : public ShaderStage {
	public:
		VolumetricCloudCS() : ShaderStage(GL_COMPUTE_SHADER, "VolumetricCloudCS") {
			addDefine("RENDER_PANORAMA", (uint32)bPanorama);
			setFilepath("sky/volumetric_clouds.glsl");
		}
	};
	DEFINE_COMPUTE_PROGRAM(Program_VolumetricCloud_Screen, VolumetricCloudCS<false>);
	DEFINE_COMPUTE_PROGRAM(Program_VolumetricCloud_Panorama, VolumetricCloudCS<true>);

	class VolumetricCloudPostFS : public ShaderStage {
	public:
		VolumetricCloudPostFS() : ShaderStage(GL_FRAGMENT_SHADER, "VolumetricCloudPostFS") {
			setFilepath("sky/volumetric_clouds_post.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_VolumetricCloudPost, FullscreenVS, VolumetricCloudPostFS);

}

namespace pathos {

	void VolumetricCloudPass::initializeResources(RenderCommandList& cmdList) {
		ubo.init<UBO_VolumetricCloud>("UBO_VolumetricCloud");
		uboPost.init<UBO_VolumetricCloudPost>("UBO_VolumetricCloudPost");

		initializeSTBN(cmdList);

		SamplerCreateParams samplerDesc{};
		samplerDesc.MIN_FILTER = GL_LINEAR;
		samplerDesc.MAG_FILTER = GL_LINEAR;
		cloudNoiseSampler = new Sampler(samplerDesc, "Sampler_CloudNoise");
		cloudNoiseSampler->createGPUResource_renderThread(cmdList);

		gRenderDevice->createFramebuffers(1, &fboPost);
		cmdList.namedFramebufferDrawBuffer(fboPost, GL_COLOR_ATTACHMENT0);
	}

	void VolumetricCloudPass::releaseResources(RenderCommandList& cmdList) {
		delete stbnTexture;

		stbnSampler->releaseGPUResource_renderThread(cmdList);
		delete stbnSampler;

		cloudNoiseSampler->releaseGPUResource_renderThread(cmdList);
		delete cloudNoiseSampler;

		gRenderDevice->deleteFramebuffers(1, &fboPost);
	}

	void VolumetricCloudPass::renderVolumetricCloud(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(VolumetricCloud);

		const bool bRenderClouds = isPassEnabled(scene);
		const bool bPanorama = cvar_cloud_panorama.getInt() != 0;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		GLuint program = 0;
		if (bPanorama) {
			program = FIND_SHADER_PROGRAM(Program_VolumetricCloud_Panorama).getGLName();
		} else {
			program = FIND_SHADER_PROGRAM(Program_VolumetricCloud_Screen).getGLName();
		}

		float resolutionScale = glm::clamp(cvar_cloud_resolution.getFloat(), 0.1f, 1.0f);
		recreateRenderTarget(cmdList, sceneContext.sceneWidth, sceneContext.sceneHeight, resolutionScale, bPanorama);

		if (!bRenderClouds) {
			float clearValues[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			pathos::clearTexture2D(
				cmdList,
				sceneContext.getVolumetricCloud(scene->frameNumber),
				renderTargetWidth,
				renderTargetHeight,
				EClearTextureFormat::RGBA16f,
				clearValues);
			return;
		}

		if (!bPanorama) {
			cmdList.textureParameteri(sceneContext.sceneDepth, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
		}

		cmdList.useProgram(program);

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
				vector3 intensity = scene->proxyList_directionalLight[0]->getIntensity();
				intensity *= std::max(0.0f, cvar_cloud_sunIntensityScale.getFloat());
				uboData.sunIntensity = vector4(intensity, 0.0f);
				uboData.sunDirection = vector4(scene->proxyList_directionalLight[0]->directionWS, 0.0f);
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
			uboData.panoramaCameraY = cvar_cloud_panoramaCameraY.getFloat();
		}
		ubo.update(cmdList, UBO_VolumetricCloud::BINDING_POINT, &uboData);

		const GLuint prevCloudTexture = sceneContext.getPrevVolumetricCloud(bPanorama ? 0 : scene->frameNumber);
		const GLuint currCloudTexture = sceneContext.getVolumetricCloud(bPanorama ? 0 : scene->frameNumber);

		cmdList.bindTextureUnit(0, sceneContext.sceneDepth);
		cmdList.bindTextureUnit(1, scene->cloud->weatherTexture->internal_getGLName());
		cmdList.bindTextureUnit(2, scene->cloud->shapeNoise->internal_getGLName());
		cmdList.bindTextureUnit(3, scene->cloud->erosionNoise->internal_getGLName());
		cmdList.bindTextureUnit(4, stbnTexture->internal_getGLName());
		cmdList.bindTextureUnit(5, prevCloudTexture);
		cmdList.bindImageTexture(6, currCloudTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, PF_volumetricCloud);

		cmdList.bindSampler(2, cloudNoiseSampler->internal_getGLName());
		cmdList.bindSampler(3, cloudNoiseSampler->internal_getGLName());
		cmdList.bindSampler(4, stbnSampler->internal_getGLName());

		const uint32 totalDispatchX = bPanorama ? (renderTargetWidth + 3) / 4 : renderTargetWidth;
		const uint32 totalDispatchY = bPanorama ? (renderTargetHeight + 3) / 4 : renderTargetHeight;
		const GLuint workGroupsX = (totalDispatchX + 15) / 16;
		const GLuint workGroupsY = (totalDispatchY + 15) / 16;
		cmdList.dispatchCompute(workGroupsX, workGroupsY, 1);

		cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		GLuint nullSamplers[] = { 0, 0, 0 };
		cmdList.bindSamplers(2, _countof(nullSamplers), nullSamplers);
	}

	void VolumetricCloudPass::renderVolumetricCloudPost(RenderCommandList& cmdList, SceneProxy* scene) {
		if (isPassEnabled(scene)) {
			SCOPED_DRAW_EVENT(VolumetricCloudPost);

			SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VolumetricCloudPost);
			auto fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

			const bool bPanorama = cvar_cloud_panorama.getInt() != 0;

			cmdList.useProgram(program.getGLName());

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboPost);
			cmdList.namedFramebufferTexture(fboPost, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);

			pathos::checkFramebufferStatus(cmdList, fboPost, "[VolumetricCloudPost] FBO is invalid");

			// Set render states
			cmdList.disable(GL_DEPTH_TEST);
			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ZERO, GL_ONE);

			cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

			// Bind shader parameters
			UBO_VolumetricCloudPost uboData;
			{
				uboData.bPanorama = (uint32)bPanorama;
				uboData.zFarPlane = pathos::getDeviceFarDepth();
			}
			uboPost.update(cmdList, UBO_VolumetricCloudPost::BINDING_POINT, &uboData);

			const GLuint currCloudTexture = sceneContext.getVolumetricCloud(bPanorama ? 0 : scene->frameNumber);

			cmdList.bindTextureUnit(0, currCloudTexture);
			cmdList.bindTextureUnit(1, sceneContext.sceneDepth);

			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			// Restore bindings
			cmdList.bindTextureUnit(0, 0);

			// Restore render states
			cmdList.disable(GL_BLEND);
			cmdList.namedFramebufferTexture(fboPost, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

	// #todo-cloud: Make STBN a system texture if needed.
	void VolumetricCloudPass::initializeSTBN(RenderCommandList& cmdList) {
		bHasValidResources = true;
		std::vector<ImageBlob*> blobs;
		for (uint32 i = 0; i < 64; ++i) {
			char buf[256];
			sprintf_s(buf,
				"resources_external/NVidiaSpatioTemporalBlueNoise/STBN/%s_%u.png",
				"stbn_scalar_2Dx1Dx1D_128x128x64x1", i);
			std::string filepath = ResourceFinder::get().find(buf);

			if (filepath.size() == 0) {
				LOG(LogError, "Run Setup.ps1 to download NVidia STBN");
				bHasValidResources = false;
				break;
			}

			ImageBlob* blob = ImageUtils::loadImage(filepath.c_str());
			blobs.push_back(blob);

			bool bValidSize = (blob->width == 128 && blob->height == 128 && blob->bpp == 8);
			if (bValidSize == false) {
				LOG(LogError, "STBN image [%u] has invalid size (not 128x128 grayscale) : width = %u, height = %u, bpp = %u", i, blob->width, blob->height, blob->bpp);
				bHasValidResources = false;
				break;
			}
		}

		if (!bHasValidResources) {
			for (auto blob : blobs) {
				cmdList.registerDeferredCleanup(blob);
			}
		} else {
			TextureCreateParams textureDesc{ 128, 128, 64, 1, GL_TEXTURE_3D, GL_R8 };
			textureDesc.imageBlobs = std::move(blobs);
			textureDesc.debugName = "Texture_NVidiaSTBN";

			stbnTexture = new Texture(textureDesc);
			stbnTexture->createGPUResource_renderThread(cmdList);

			SamplerCreateParams samplerDesc{};
			samplerDesc.MIN_FILTER = GL_NEAREST;
			samplerDesc.MAG_FILTER = GL_NEAREST;

			stbnSampler = new Sampler(samplerDesc, "Sampler_NVidiaSTBN");
			stbnSampler->createGPUResource_renderThread(cmdList);
		}
	}

	void VolumetricCloudPass::recreateRenderTarget(RenderCommandList& cmdList, uint32 inScreenWidth, uint32 inScreenHeight, float inResolutionScale, bool bPanorama)
	{
		CHECKF(inScreenWidth != 0 && inScreenHeight != 0, "Invalid size for cloud render target");

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const uint32 targetWidth = bPanorama ? (uint32)cvar_cloud_panoramaWidth.getInt() : (uint32)(inScreenWidth * inResolutionScale);
		const uint32 targetHeight = bPanorama ? (uint32)cvar_cloud_panoramaHeight.getInt() : (uint32)(inScreenHeight * inResolutionScale);

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

	bool VolumetricCloudPass::isPassEnabled(const SceneProxy* scene) const {
		return bHasValidResources && scene->isVolumetricCloudValid() && (cvar_enable_volClouds.getInt() != 0);
	}

}
