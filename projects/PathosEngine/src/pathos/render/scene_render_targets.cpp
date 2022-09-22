#include "scene_render_targets.h"
#include "pathos/render/render_device.h"
#include "badger/assertion/assertion.h"
#include "../util/log.h"

namespace pathos {

	SceneRenderTargets::SceneRenderTargets() {}

	SceneRenderTargets::~SceneRenderTargets() {
		CHECK(destroyed);
	}

	void SceneRenderTargets::reallocSceneTextures(RenderCommandList& cmdList, uint32 width, uint32 height)
	{
		CHECK(width > 0 && height > 0);

		destroyed = false;

		const bool bResolutionChanged = sceneWidth != width || sceneHeight != height;
		sceneWidth = width;
		sceneHeight = height;

		if (!bResolutionChanged) {
			return;
		}

		// #todo-rendertarget: Merge reallocTexture2D and reallocTexture2DMips
		auto reallocTexture2D = [&cmdList](GLuint& texture, GLenum format, uint32 width, uint32 height, char* objectLabel) -> void {
			if (texture != 0) {
				cmdList.deleteTextures(1, &texture);
			}
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &texture);
			cmdList.textureStorage2D(texture, 1, format, width, height);
			cmdList.bindTexture(GL_TEXTURE_2D, texture);
			cmdList.objectLabel(GL_TEXTURE, texture, -1, objectLabel);
		};
		auto reallocTexture2DMips = [&cmdList](GLuint& texture, GLenum format, uint32 width, uint32 height, uint32 numMips, char* objectLabel) -> void {
			if (texture != 0) {
				cmdList.deleteTextures(1, &texture);
			}
			gRenderDevice->createTextures(GL_TEXTURE_2D, 1, &texture);
			cmdList.textureStorage2D(texture, numMips, format, width, height);
			cmdList.bindTexture(GL_TEXTURE_2D, texture);
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
			cmdList.bindTexture(GL_TEXTURE_2D_ARRAY, texture);
			cmdList.objectLabel(GL_TEXTURE, texture, -1, objectLabel);
		};

		// #todo-rendertarget: Switch PF_sceneColor to rgba16f?
		static constexpr GLenum PF_sceneColor = GL_RGBA32F;
		static constexpr GLenum PF_sceneDepth = GL_DEPTH32F_STENCIL8;
		reallocTexture2D(sceneFinal, PF_sceneColor, sceneWidth, sceneHeight, "sceneFinal");
		reallocTexture2D(sceneColor, PF_sceneColor, sceneWidth, sceneHeight, "sceneColor");
		reallocTexture2D(sceneColorHalfRes, PF_sceneColor, sceneWidth / 2, sceneHeight / 2, "sceneColor");
		reallocTexture2D(sceneDepth, PF_sceneDepth, sceneWidth, sceneHeight, "sceneDepth");

		// Screen space reflection
		{
			// HiZ
			constexpr GLenum PF_HiZ = GL_RG32F;
			sceneDepthHiZMipmapCount = static_cast<uint32>(1 + floor(log2(std::max(sceneWidth, sceneHeight))));
			reallocTexture2DMips(sceneDepthHiZ, PF_HiZ, sceneWidth, sceneHeight, sceneDepthHiZMipmapCount, "sceneDepthHiZ");
			reallocTexture2DViews(sceneDepthHiZViews, sceneDepthHiZMipmapCount, sceneDepthHiZ, PF_HiZ, "view_sceneDepthHiZMip");
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(sceneDepthHiZ, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// Preintegration
			constexpr GLenum PF_preintegration = GL_R8;
			ssrPreintegrationMipmapCount = static_cast<uint32>(1 + floor(log2(std::max(sceneWidth, sceneHeight))));
			reallocTexture2DMips(ssrPreintegration, PF_preintegration, sceneWidth, sceneHeight, ssrPreintegrationMipmapCount, "ssrPreintegration");
			reallocTexture2DViews(ssrPreintegrationViews, ssrPreintegrationMipmapCount, ssrPreintegration, PF_preintegration, "ssrPreintegrationMip");
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(ssrPreintegration, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// Ray tracing
			constexpr GLenum PF_raytracing = GL_RGB16F;
			reallocTexture2D(ssrRayTracing, PF_raytracing, sceneWidth, sceneHeight, "ssrRayTracing");
			// Preconvolution (NOTE: starts at half res)
			constexpr GLenum PF_preconvolution = GL_RGB16F;
			const uint32 preconvWidth = sceneWidth / 2, preconvHeight = sceneHeight / 2;
			ssrPreconvolutionMipmapCount = static_cast<uint32>(1 + floor(log2(std::max(preconvWidth, preconvHeight))));
			reallocTexture2DMips(ssrPreconvolution, PF_preconvolution, preconvWidth, preconvHeight, ssrPreconvolutionMipmapCount, "ssrPreconvolution");
			reallocTexture2DMips(ssrPreconvolutionTemp, PF_preconvolution, preconvWidth, preconvHeight, ssrPreconvolutionMipmapCount, "ssrPreconvolutionTemp");
			reallocTexture2DViews(ssrPreconvolutionViews, ssrPreconvolutionMipmapCount, ssrPreconvolution, PF_preconvolution, "ssrPreconvolutionMip");
			reallocTexture2DViews(ssrPreconvolutionTempViews, ssrPreconvolutionMipmapCount, ssrPreconvolutionTemp, PF_preconvolution, "ssrPreconvolutionTempMip");
		}

		// CSM
		reallocTexture2DArray(cascadedShadowMap, GL_DEPTH_COMPONENT32F, csmWidth, csmHeight, numCascades, "CascadedShadowMap");
		cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(cascadedShadowMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// gbuffer
		if (useGBuffer) {
			reallocGBuffers(cmdList, bResolutionChanged);
		}

		// god ray
		reallocTexture2D(godRaySource, GL_RGBA32F, sceneWidth, sceneHeight, "godRaySource");
		reallocTexture2D(godRayResult, GL_RGBA32F, sceneWidth / 2, sceneHeight / 2, "godRayResult");
		reallocTexture2D(godRayResultTemp, GL_RGBA32F, sceneWidth / 2, sceneHeight / 2, "godRayResultTemp");
		cmdList.textureParameteri(godRayResultTemp, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(godRayResultTemp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// depth of field
		constexpr GLenum PF_dofSubsum = GL_RGBA32F;
		reallocTexture2D(dofSubsum0, PF_dofSubsum, sceneHeight, sceneWidth, "depthOfField_subsum0");
		reallocTexture2D(dofSubsum1, PF_dofSubsum, sceneWidth, sceneHeight, "depthOfField_subsum1");

		// bloom
		constexpr GLenum PF_bloom = GL_RGBA16F;
		reallocTexture2D(sceneBloomSetup, PF_bloom, sceneWidth / 2, sceneHeight / 2, "sceneBloomSetup");
		cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		cmdList.textureParameteri(sceneBloomSetup, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		sceneBloomChainMipCount = std::min(5u, static_cast<uint32>(floor(log2(std::max(sceneWidth / 2, sceneHeight / 2))) + 1));
		reallocTexture2DMips(sceneBloomChain, PF_bloom, sceneWidth / 2, sceneHeight / 2, sceneBloomChainMipCount, "sceneBloomChain");
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

		// tone mapping
		reallocTexture2D(toneMappingResult, GL_RGBA16F, sceneWidth, sceneHeight, "toneMappingResult");

		// ssao
		reallocTexture2D(ssaoHalfNormalAndDepth, GL_RGBA16F, sceneWidth / 2, sceneHeight / 2, "ssaoHalfNormalAndDepth");
		reallocTexture2D(ssaoMap, GL_R16F, sceneWidth / 2, sceneHeight / 2, "ssaoMap");
		reallocTexture2D(ssaoMapTemp, GL_R16F, sceneWidth / 2, sceneHeight / 2, "ssaoMapTemp");
	}

	void SceneRenderTargets::freeSceneTextures(RenderCommandList& cmdList)
	{
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

#define safe_release(x) if(x != 0) { textures.push_back(x); x = 0; }
		safe_release(sceneFinal);
		safe_release(sceneColor);
		safe_release(sceneDepth);
		safe_release(sceneBloomChain);
		safe_release(sceneDepthHiZ);
		safe_release(ssrPreintegration);
		safe_release(ssrRayTracing);
		safe_release(ssrPreconvolution);
		safe_release(ssrPreconvolutionTemp);
		safe_release(volumetricCloudA);
		safe_release(volumetricCloudB);
		safe_release(cascadedShadowMap);
		safe_release(omniShadowMaps);
		safe_release(gbufferA);
		safe_release(gbufferB);
		safe_release(gbufferC);
		safe_release(godRaySource);
		safe_release(godRayResult);
		safe_release(godRayResultTemp);
		safe_release(dofSubsum0);
		safe_release(dofSubsum1);
		safe_release(toneMappingResult);
		safe_release(ssaoHalfNormalAndDepth);
		safe_release(ssaoMap);
		safe_release(ssaoMapTemp);
#undef safe_release

		gRenderDevice->deleteTextures((GLsizei)textures.size(), textures.data());

		destroyed = true;
	}

	void SceneRenderTargets::reallocOmniShadowMaps(RenderCommandList& cmdList, uint32 numPointLights, uint32 width, uint32 height)
	{
		if (omniShadowMaps != 0) {
			cmdList.deleteTextures(1, &omniShadowMaps);
			omniShadowMaps = 0;
		}
		if (numPointLights > 0) {
			gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &omniShadowMaps);
			cmdList.textureStorage3D(
				omniShadowMaps,
				1,
				GL_DEPTH_COMPONENT32F,
				width,
				height,
				numPointLights * 6);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			cmdList.textureParameteri(omniShadowMaps, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			cmdList.bindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, omniShadowMaps);
			cmdList.objectLabel(GL_TEXTURE, omniShadowMaps, -1, "OmniShadowMaps");
		}
	}

	void SceneRenderTargets::reallocGBuffers(RenderCommandList& cmdList, bool bResolutionChanged)
	{
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

		// #todo-renderstate: Use sampler object
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

}
