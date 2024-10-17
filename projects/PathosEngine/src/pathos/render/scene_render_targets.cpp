#include "scene_render_targets.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/texture.h"
#include "pathos/render/postprocessing/super_res.h"
#include "pathos/scene/reflection_probe_component.h"
#include "pathos/console.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

static uint32 calcCubemapNumMips(uint32 defaultSize, uint32 minSize) {
	uint32 num = 1;
	while (defaultSize >= minSize) {
		defaultSize /= 2;
		num += 1;
	}
	return num;
}

static uint32 calcTexture2DMaxMipCount(uint32 width, uint32 height) {
	return (uint32)(1 + std::floor(std::log2(std::max(width, height))));
}

namespace pathos {

	static ConsoleVariable<int32> cvar_resolutionScale("r.resolution_scale", 100, "Controls screen resolution percentage");

	SceneRenderTargets::SceneRenderTargets() {}

	SceneRenderTargets::~SceneRenderTargets() {
		CHECK(bDestroyed);
	}

	void SceneRenderTargets::reallocSceneTextures(
		RenderCommandList& cmdList,
		SceneProxySource sceneProxySource,
		uint32 newWidth,
		uint32 newHeight,
		bool bEnableResolutionScaling)
	{
		CHECK(newWidth > 0 && newHeight > 0);
		const bool bLightProbeRendering = isLightProbeRendering(sceneProxySource);

		bDestroyed = false;

		unscaledSceneWidth = newWidth;
		unscaledSceneHeight = newHeight;

		// Screen percentage
		if (bEnableResolutionScaling && cvar_resolutionScale.getInt() != 100) {
			float resolutionScale = badger::clamp(0.5f, 0.01f * (float)cvar_resolutionScale.getInt(), 2.0f);
			newWidth = (uint32)((float)newWidth * resolutionScale);
			newHeight = (uint32)((float)newHeight * resolutionScale);
		}

		sceneWidthSuperRes = newWidth;
		sceneHeightSuperRes = newHeight;

		// Super resolution
		bool bSuperResolution = (bEnableResolutionScaling && pathos::getSuperResolutionScaleFactor() != 1.0f);
		if (bSuperResolution) {
			float scaleFactor = pathos::getSuperResolutionScaleFactor();
			newWidth = (uint32)((float)newWidth / scaleFactor);
			newHeight = (uint32)((float)newHeight / scaleFactor);
		}

		const bool bResolutionChanged = (sceneWidth != newWidth) || (sceneHeight != newHeight);
		sceneWidth = newWidth;
		sceneHeight = newHeight;

		if (!bResolutionChanged) {
			return;
		}

		auto reallocTexture2D = [&cmdList](GLuint& texture, GLenum format, uint32 width, uint32 height, uint32 numMips, char* objectLabel) -> void {
			if (texture != 0) {
				cmdList.deleteTextures(1, &texture);
			}
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &texture);
			cmdList.textureStorage2D(texture, numMips, format, width, height);
			cmdList.objectLabel(GL_TEXTURE, texture, -1, objectLabel);
		};
		// Create texture views for the original texture from mip 0 to mip (numMips-1)
		auto reallocTexture2DViews = [&cmdList](std::vector<GLuint>& views, uint32 numMips, GLuint original, GLenum format, char* objectLabel) -> void {
			if (views.size() > 0) {
				gRenderDevice->deleteTextures((GLsizei)views.size(), views.data());
			}
			views.resize(numMips);
			gRenderDevice->genTextures(numMips, views.data());
			for (uint32 i = 0; i < numMips; ++i) {
				GLuint targetView = views[i];
				std::string targetName = objectLabel + std::to_string(i);
				cmdList.textureView(targetView, GL_TEXTURE_2D, original, format, (GLuint)i, 1, 0, 1);
				cmdList.objectLabel(GL_TEXTURE, targetView, -1, targetName.c_str());
			}
		};
		auto reallocTexture2DArray = [&cmdList](GLuint& texture, GLenum format, uint32 width, uint32 height, uint32 numLayers, char* objectLabel) -> void {
			if (texture != 0) {
				cmdList.deleteTextures(1, &texture);
			}
			gRenderDevice->createTextures(GL_TEXTURE_2D_ARRAY, 1, &texture);
			cmdList.textureStorage3D(texture, 1, format, width, height, numLayers);
			cmdList.objectLabel(GL_TEXTURE, texture, -1, objectLabel);
		};
		auto reallocTextureCube = [&cmdList](GLuint& texture, GLenum format, uint32 size, uint32 numMips, char* objectLabel) -> void {
			if (texture != 0) {
				cmdList.deleteTextures(1, &texture);
			}
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);
			cmdList.textureStorage2D(texture, numMips, format, size, size);
			cmdList.objectLabel(GL_TEXTURE, texture, -1, objectLabel);
		};
		auto reallocTextureCubeArray = [&cmdList](GLuint& texture, GLenum format, uint32 size, uint32 numCubemaps, uint32 numMips, char* objectLabel) -> void {
			if (texture != 0) {
				cmdList.deleteTextures(1, &texture);
			}
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &texture);
			cmdList.textureStorage3D(texture, numMips, format, size, size, numCubemaps * 6);
			cmdList.objectLabel(GL_TEXTURE, texture, -1, objectLabel);
		};

		//////////////////////////////////////////////////////////////////////////
		// Textures that are independent of screen resolution.
		// They don't need to be recreated on window resize. (should be not recreated)
		
		// reallocDirectionalShadowMaps() is called from shadow_directional.cpp
		// reallocOmniShadowMaps() is called from shadow_omni.cpp

		if (sceneProxySource == SceneProxySource::MainScene || sceneProxySource == SceneProxySource::SceneCapture) {
			if (localSpecularIBLs == 0) {
				reallocTextureCubeArray(localSpecularIBLs, GL_RGBA16F, pathos::reflectionProbeCubemapSize, pathos::reflectionProbeMaxCount, pathos::reflectionProbeNumMips, "LocalSpecularIBLs");
			}
			if (skyIrradianceMap == 0) {
				reallocSkyIrradianceMap(cmdList);
			}
			// One of sky passes will invoke reallocSkyPrefilterMap()

			// Auto exposure (histogram)
			if (luminanceFromHistogram == 0) {
				reallocTexture2D(luminanceFromHistogram, GL_R32F, 1, 1, 1, "luminanceFromHistogram");
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Before super resolution

		// GBuffers
		if (useGBuffer) {
			reallocGBuffers(cmdList, bResolutionChanged);
		}

		// God ray
		if (bLightProbeRendering == false) {
			reallocTexture2D(godRaySource,     GL_RGBA16F, sceneWidth,     sceneHeight,     1, "godRaySource");
			reallocTexture2D(godRayResult,     GL_RGBA16F, sceneWidth / 2, sceneHeight / 2, 1, "godRayResult");
			reallocTexture2D(godRayResultTemp, GL_RGBA16F, sceneWidth / 2, sceneHeight / 2, 1, "godRayResultTemp");
			cmdList.textureParameteri(godRayResultTemp, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(godRayResultTemp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		// SSAO
		reallocTexture2D(ssaoHalfNormalAndDepth, GL_RGBA16F, sceneWidth / 2, sceneHeight / 2, 1, "ssaoHalfNormalAndDepth");
		reallocTexture2D(ssaoMap,                GL_R16F,    sceneWidth / 2, sceneHeight / 2, 1, "ssaoMap");
		reallocTexture2D(ssaoMapTemp,            GL_R16F,    sceneWidth / 2, sceneHeight / 2, 1, "ssaoMapTemp");

		// sceneColor, sceneDepth
		static constexpr GLenum PF_sceneColor = GL_RGBA16F;
		static constexpr GLenum PF_sceneDepth = GL_DEPTH32F_STENCIL8;
		reallocTexture2D(sceneColor,        PF_sceneColor, sceneWidth,     sceneHeight,     1, "sceneColor");
		reallocTexture2D(sceneColorHalfRes, PF_sceneColor, sceneWidth / 2, sceneHeight / 2, 1, "sceneColorHalfRes");
		reallocTexture2D(sceneDepth,        PF_sceneDepth, sceneWidth,     sceneHeight,     1, "sceneDepth");

		// Auto exposure (average log luminance)
		sceneLuminanceSize = 1024;
		while (sceneLuminanceSize > sceneWidth || sceneLuminanceSize > sceneHeight) sceneLuminanceSize /= 2;
		sceneLuminanceMipCount = calcTexture2DMaxMipCount(sceneLuminanceSize, sceneLuminanceSize);
		reallocTexture2D(sceneLuminance, GL_R16F, sceneLuminanceSize, sceneLuminanceSize, sceneLuminanceMipCount, "sceneLuminance");

		// Screen space reflection
		if (bLightProbeRendering == false) {
			// HiZ
			constexpr GLenum PF_HiZ = GL_RG32F;
			sceneDepthHiZMipmapCount = calcTexture2DMaxMipCount(sceneWidth, sceneHeight);
			reallocTexture2D(sceneDepthHiZ, PF_HiZ, sceneWidth, sceneHeight, sceneDepthHiZMipmapCount, "sceneDepthHiZ");
			reallocTexture2DViews(sceneDepthHiZViews, sceneDepthHiZMipmapCount, sceneDepthHiZ, PF_HiZ, "view_sceneDepthHiZMip");
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// Preintegration
			constexpr GLenum PF_preintegration = GL_R8;
			ssrPreintegrationMipmapCount = calcTexture2DMaxMipCount(sceneWidth, sceneHeight);
			reallocTexture2D(ssrPreintegration, PF_preintegration, sceneWidth, sceneHeight, ssrPreintegrationMipmapCount, "ssrPreintegration");
			reallocTexture2DViews(ssrPreintegrationViews, ssrPreintegrationMipmapCount, ssrPreintegration, PF_preintegration, "ssrPreintegrationMip");
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// Ray tracing
			constexpr GLenum PF_raytracing = GL_RGB16F;
			reallocTexture2D(ssrRayTracing, PF_raytracing, sceneWidth, sceneHeight, 1, "ssrRayTracing");
			// Preconvolution (NOTE: starts at half res)
			constexpr GLenum PF_preconvolution = GL_RGB16F;
			const uint32 preconvWidth = sceneWidth / 2, preconvHeight = sceneHeight / 2;
			ssrPreconvolutionMipmapCount = calcTexture2DMaxMipCount(preconvWidth, preconvHeight);
			reallocTexture2D(ssrPreconvolution, PF_preconvolution, preconvWidth, preconvHeight, ssrPreconvolutionMipmapCount, "ssrPreconvolution");
			reallocTexture2D(ssrPreconvolutionTemp, PF_preconvolution, preconvWidth, preconvHeight, ssrPreconvolutionMipmapCount, "ssrPreconvolutionTemp");
			reallocTexture2DViews(ssrPreconvolutionViews, ssrPreconvolutionMipmapCount, ssrPreconvolution, PF_preconvolution, "ssrPreconvolutionMip");
			reallocTexture2DViews(ssrPreconvolutionTempViews, ssrPreconvolutionMipmapCount, ssrPreconvolutionTemp, PF_preconvolution, "ssrPreconvolutionTempMip");
		}

		// Bloom
		if (bLightProbeRendering == false) {
			constexpr GLenum PF_bloom = GL_RGBA16F;
			reallocTexture2D(sceneBloomSetup, PF_bloom, sceneWidth / 2, sceneHeight / 2, 1, "sceneBloomSetup");
			cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			sceneBloomChainMipCount = std::min(5u, static_cast<uint32>(floor(log2(std::max(sceneWidth / 2, sceneHeight / 2))) + 1));
			reallocTexture2D(sceneBloomChain, PF_bloom, sceneWidth / 2, sceneHeight / 2, sceneBloomChainMipCount, "sceneBloomChain");
			reallocTexture2DViews(sceneBloomChainViews, sceneBloomChainMipCount, sceneBloomChain, PF_bloom, "view_sceneBloomChain");
			cmdList.textureParameteri(sceneBloomChain, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(sceneBloomChain, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(sceneBloomChain, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			cmdList.textureParameteri(sceneBloomChain, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			for (uint32 i = 0; i < sceneBloomChainMipCount; ++i) {
				cmdList.textureParameteri(sceneBloomChainViews[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				cmdList.textureParameteri(sceneBloomChainViews[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				cmdList.textureParameteri(sceneBloomChainViews[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				cmdList.textureParameteri(sceneBloomChainViews[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}
		}

		// Anti-aliasing
		static constexpr GLenum PF_sceneColorAA = GL_RGBA16F;
		static constexpr GLenum PF_velocityMap = GL_RG16F;
		reallocTexture2D(sceneColorAA,      PF_sceneColorAA, sceneWidth, sceneHeight, 1, "sceneColorAA");
		reallocTexture2D(sceneColorHistory, PF_sceneColorAA, sceneWidth, sceneHeight, 1, "sceneColorHistory");
		reallocTexture2D(velocityMap,       PF_velocityMap,  sceneWidth, sceneHeight, 1, "velocityMap");

		// Tone mapping
		if (bLightProbeRendering == false) {
			reallocTexture2D(sceneColorToneMapped, GL_RGBA16F, sceneWidth, sceneHeight, 1, "sceneColorToneMapped");
		}

		//////////////////////////////////////////////////////////////////////////
		// After super resolution

		uint32 sceneWidthBackup = sceneWidth, sceneHeightBackup = sceneHeight;
		sceneWidth = sceneWidthSuperRes;
		sceneHeight = sceneHeightSuperRes;

		// Super resolution
		static constexpr GLenum PF_sceneColorSuperRes = GL_RGBA16F;
		reallocTexture2D(sceneColorUpscaledTemp, PF_sceneColorAA, sceneWidth, sceneHeight, 1, "sceneColorUpscaledTemp");
		reallocTexture2D(sceneColorUpscaled,     PF_sceneColorAA, sceneWidth, sceneHeight, 1, "sceneColorUpscaled");

		// Depth of field
		if (bLightProbeRendering == false) {
			constexpr GLenum PF_dofSubsum = GL_RGBA32F;
			reallocTexture2D(sceneColorDoFInput, PF_dofSubsum, sceneWidth, sceneHeight, 1, "DoF_sceneColor32f");
			reallocTexture2D(dofSubsum0,         PF_dofSubsum, sceneHeight, sceneWidth, 1, "depthOfField_subsum0");
			reallocTexture2D(dofSubsum1,         PF_dofSubsum, sceneWidth, sceneHeight, 1, "depthOfField_subsum1");
		}

		// sceneFinal
		static constexpr GLenum PF_sceneFinal = GL_RGBA16F;
		reallocTexture2D(sceneFinal, PF_sceneFinal, sceneWidth, sceneHeight, 1, "sceneFinal");

		sceneWidth = sceneWidthBackup;
		sceneHeight = sceneHeightBackup;
	}

	void SceneRenderTargets::freeSceneTextures(RenderCommandList& cmdList) {
		std::vector<GLuint> textures;
		textures.reserve(64);

		// Delete texture views first
		auto releaseViews = [&](const std::vector<GLuint>& views) -> void {
			uint32 n = (uint32)views.size();
			for (uint32 i = 0; i < n; ++i) {
				textures.push_back(views[i]);
			}
		};
		releaseViews(sceneBloomChainViews);
		releaseViews(sceneDepthHiZViews);
		releaseViews(ssrPreintegrationViews);
		releaseViews(ssrPreconvolutionViews);
		releaseViews(ssrPreconvolutionTempViews);

		// #todo-renderer: Implement RT pool and release all automatically.
#define safe_release(x) if(x != 0) { textures.push_back(x); x = 0; }
		safe_release(sceneColor);
		safe_release(sceneColorHalfRes);
		safe_release(sceneDepth);
		safe_release(sceneLuminance);
		safe_release(luminanceFromHistogram);
		safe_release(sceneColorAA);
		safe_release(sceneColorHistory);
		safe_release(velocityMap);
		safe_release(sceneColorUpscaledTemp);
		safe_release(sceneColorUpscaled);
		safe_release(sceneFinal);
		safe_release(sceneDepthHiZ);
		safe_release(ssrPreintegration);
		safe_release(ssrRayTracing);
		safe_release(ssrPreconvolution);
		safe_release(ssrPreconvolutionTemp);
		safe_release(volumetricCloudA);
		safe_release(volumetricCloudB);
		safe_release(cascadedShadowMap);
		safe_release(omniShadowMaps);
		safe_release(localSpecularIBLs);
		safe_release(skyIrradianceMap);
		safe_release(skyPrefilteredMap);
		safe_release(gbufferA);
		safe_release(gbufferB);
		safe_release(gbufferC);
		safe_release(godRaySource);
		safe_release(godRayResult);
		safe_release(godRayResultTemp);
		safe_release(dofSubsum0);
		safe_release(dofSubsum1);
		safe_release(sceneBloomSetup);
		safe_release(sceneBloomChain);
		safe_release(sceneColorToneMapped);
		safe_release(ssaoHalfNormalAndDepth);
		safe_release(ssaoMap);
		safe_release(ssaoMapTemp);
#undef safe_release

		gRenderDevice->deleteTextures((GLsizei)textures.size(), textures.data());

		bDestroyed = true;
	}

	void SceneRenderTargets::reallocDirectionalShadowMaps(RenderCommandList& cmdList) {
		if (actualCsmCount == csmCount && cascadedShadowMap != 0) {
			return;
		}
		actualCsmCount = csmCount;

		if (cascadedShadowMap != 0) {
			cmdList.deleteTextures(1, &cascadedShadowMap);
			cascadedShadowMap = 0;
		}
		if (csmCount > 0) {
			gRenderDevice->createTextures(GL_TEXTURE_2D_ARRAY, 1, &cascadedShadowMap);
			cmdList.textureStorage3D(cascadedShadowMap, 1, GL_DEPTH_COMPONENT32F, csmWidth, csmHeight, csmCount);
			cmdList.objectLabel(GL_TEXTURE, cascadedShadowMap, -1, "CascadedShadowMap");

			cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
	}

	void SceneRenderTargets::reallocOmniShadowMaps(RenderCommandList& cmdList, uint32 numPointLights, uint32 shadowMapSize) {
		if (omniShadowMapLayerCount == (numPointLights * 6) && omniShadowMapSize == shadowMapSize && omniShadowMaps != 0) {
			return;
		}
		omniShadowMapLayerCount = numPointLights * 6;
		omniShadowMapSize = shadowMapSize;

		if (omniShadowMaps != 0) {
			cmdList.deleteTextures(1, &omniShadowMaps);
			omniShadowMaps = 0;
		}
		if (omniShadowMapLayerCount > 0) {
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &omniShadowMaps);
			cmdList.textureStorage3D(omniShadowMaps, 1 /*mip count*/, GL_DEPTH_COMPONENT32F, omniShadowMapSize, omniShadowMapSize, omniShadowMapLayerCount);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.objectLabel(GL_TEXTURE, omniShadowMaps, -1, "OmniShadowMaps");
		}
	}

	void SceneRenderTargets::reallocGBuffers(RenderCommandList& cmdList, bool bResolutionChanged) {
		if (bResolutionChanged) {
			if (gbufferA != 0) cmdList.deleteTextures(1, &gbufferA);
			if (gbufferB != 0) cmdList.deleteTextures(1, &gbufferB);
			if (gbufferC != 0) cmdList.deleteTextures(1, &gbufferC);
		}

		GLuint gbuffers[3];
		gRenderDevice->createTextures(GL_TEXTURE_2D, 3, gbuffers);
		gbufferA = gbuffers[0];
		gbufferB = gbuffers[1];
		gbufferC = gbuffers[2];

		cmdList.textureStorage2D(gbufferA, 1, GL_RGBA32UI, sceneWidth, sceneHeight);
		cmdList.textureStorage2D(gbufferB, 1, GL_RGBA32F, sceneWidth, sceneHeight);
		cmdList.textureStorage2D(gbufferC, 1, GL_RGBA32UI, sceneWidth, sceneHeight);

		// #todo-renderer: Use sampler object
		cmdList.textureParameteri(gbufferA, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.textureParameteri(gbufferA, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		cmdList.textureParameteri(gbufferB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.textureParameteri(gbufferB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		cmdList.textureParameteri(gbufferC, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.textureParameteri(gbufferC, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		cmdList.objectLabel(GL_TEXTURE, gbufferA, -1, "gbufferA");
		cmdList.objectLabel(GL_TEXTURE, gbufferB, -1, "gbufferB");
		cmdList.objectLabel(GL_TEXTURE, gbufferC, -1, "gbufferC");
	}

	void SceneRenderTargets::reallocSkyIrradianceMap(RenderCommandList& cmdList) {
		if (skyIrradianceMap != 0) {
			cmdList.deleteTextures(1, &skyIrradianceMap);
		}
		gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, &skyIrradianceMap);
		cmdList.textureStorage2D(skyIrradianceMap, 1, GL_RGBA16F, SKY_IRRADIANCE_MAP_SIZE, SKY_IRRADIANCE_MAP_SIZE);
		cmdList.objectLabel(GL_TEXTURE, skyIrradianceMap, -1, "SkyIrradianceMap");
	}

	void SceneRenderTargets::reallocSkyPrefilterMap(RenderCommandList& cmdList, uint32 cubemapSize) {
		CHECKF(cubemapSize > 0, "cubemapSize is zero");
		if (skyPrefilteredMap != 0 && skyPrefilterMapSize != cubemapSize) {
			gRenderDevice->deleteTextures(1, &skyPrefilteredMap);
			skyPrefilteredMap = 0;
		}

		skyPrefilterMapMipCount = calcCubemapNumMips(cubemapSize, SKY_PREFILTER_MAP_MIN_SIZE);
		skyPrefilterMapMipCount = std::min(skyPrefilterMapMipCount, SKY_PREFILTER_MAP_MAX_NUM_MIPS);
		skyPrefilterMapSize = cubemapSize;
		if (skyPrefilteredMap == 0) {
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, &skyPrefilteredMap);
			cmdList.textureStorage2D(
				skyPrefilteredMap,
				skyPrefilterMapMipCount,
				GL_RGBA16F,
				cubemapSize,
				cubemapSize);
			cmdList.objectLabel(GL_TEXTURE, skyPrefilteredMap, -1, "Texture_SkyPrefilterMap");
		}
	}

	GLuint SceneRenderTargets::getSkyIrradianceMapWithFallback() const {
		return (skyIrradianceMap != 0) ? skyIrradianceMap : gEngine->getSystemTextureCubeBlack()->internal_getGLName();
	}

	GLuint SceneRenderTargets::getSkyPrefilterMapWithFallback() const {
		return (skyPrefilteredMap != 0) ? skyPrefilteredMap : gEngine->getSystemTextureCubeBlack()->internal_getGLName();
	}

	uint32 SceneRenderTargets::getSkyPrefilterMapMipCount() const {
		return (skyPrefilteredMap != 0) ? skyPrefilterMapMipCount : 1;
	}

}
