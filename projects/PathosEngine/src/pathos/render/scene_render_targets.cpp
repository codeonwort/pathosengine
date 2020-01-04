#include "scene_render_targets.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	SceneRenderTargets::SceneRenderTargets()
		: sceneWidth(0)
		, sceneHeight(0)
		, sceneFinal(0)
		, sceneColor(0)
		, sceneDepth(0)
		, cascadedShadowMap(0)
		, useGBuffer(false)
		, gbufferA(0)
		, gbufferB(0)
		, gbufferC(0)
		, godRaySource(0)
		, godRayResult(0)
		, godRayResultTemp(0)
		, dofSubsum0(0)
		, dofSubsum1(0)
		, sceneBloom(0)
		, sceneBloomTemp(0)
		, toneMappingResult(0)
	{
	}

	SceneRenderTargets::~SceneRenderTargets()
	{
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

		auto reallocTexture2D = [&cmdList](GLuint& texture, GLenum format, uint32 width, uint32 height, char* objectLabel) -> void {
			if (texture != 0) {
				cmdList.deleteTextures(1, &texture);
			}
			cmdList.createTextures(GL_TEXTURE_2D, 1, &texture);
			cmdList.textureStorage2D(texture, 1, format, width, height);
			cmdList.bindTexture(GL_TEXTURE_2D, texture);
			cmdList.objectLabel(GL_TEXTURE, texture, -1, objectLabel);
		};

		reallocTexture2D(sceneFinal, GL_RGBA32F, sceneWidth, sceneHeight, "sceneFinal");
		reallocTexture2D(sceneColor, GL_RGBA32F, sceneWidth, sceneHeight, "sceneColor");
		reallocTexture2D(sceneDepth, GL_DEPTH24_STENCIL8, sceneWidth, sceneHeight, "sceneDepth");

		// CSM
		reallocTexture2D(cascadedShadowMap, GL_DEPTH_COMPONENT32F, csmWidth * numCascades, csmHeight, "cascadedShadowMap");
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
		reallocTexture2D(godRaySource, GL_RGBA32F, sceneWidth / 2, sceneHeight / 2, "godRaySource");
		reallocTexture2D(godRayResult, GL_RGBA32F, sceneWidth / 2, sceneHeight / 2, "godRayResult");
		reallocTexture2D(godRayResultTemp, GL_RGBA32F, sceneWidth / 2, sceneHeight / 2, "godRayResultTemp");
		cmdList.textureParameteri(godRayResultTemp, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(godRayResultTemp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// depth of field
		reallocTexture2D(dofSubsum0, GL_RGBA32F, sceneHeight, sceneWidth, "depthOfField_subsum0");
		reallocTexture2D(dofSubsum1, GL_RGBA32F, sceneWidth, sceneHeight, "depthOfField_subsum1");

		// bloom
		reallocTexture2D(sceneBloom, GL_RGBA32F, sceneWidth, sceneHeight, "sceneBloom");
		cmdList.textureParameteri(sceneBloom, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.textureParameteri(sceneBloom, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		reallocTexture2D(sceneBloomTemp, GL_RGBA32F, sceneWidth, sceneHeight, "sceneBloomTemp");
		cmdList.textureParameteri(sceneBloomTemp, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.textureParameteri(sceneBloomTemp, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// tone mapping
		reallocTexture2D(toneMappingResult, GL_RGBA32F, sceneWidth, sceneHeight, "toneMappingResult");
		cmdList.textureParameteri(toneMappingResult, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		cmdList.textureParameteri(toneMappingResult, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// ssao
		reallocTexture2D(ssaoHalfNormalAndDepth, GL_RGBA32F, sceneWidth / 2, sceneHeight / 2, "ssaoHalfNormalAndDepth");
		reallocTexture2D(ssaoMap, GL_R32F, sceneWidth / 2, sceneHeight / 2, "ssaoMap");
		reallocTexture2D(ssaoMapTemp, GL_R32F, sceneWidth / 2, sceneHeight / 2, "ssaoMapTemp");
	}

	void SceneRenderTargets::freeSceneTextures(RenderCommandList& cmdList)
	{
#define safe_release(x) if(x != 0) { cmdList.deleteTextures(1, &x); x = 0; }
		safe_release(sceneFinal);
		safe_release(sceneColor);
		safe_release(sceneDepth);
		safe_release(cascadedShadowMap);
		safe_release(gbufferA);
		safe_release(gbufferB);
		safe_release(gbufferC);
		safe_release(godRaySource);
		safe_release(godRayResult);
		safe_release(godRayResultTemp);
		safe_release(dofSubsum0);
		safe_release(dofSubsum1);
		safe_release(sceneBloom);
		safe_release(sceneBloomTemp);
		safe_release(toneMappingResult);
		safe_release(ssaoHalfNormalAndDepth);
		safe_release(ssaoMap);
		safe_release(ssaoMapTemp);
#undef safe_release

		destroyed = true;
	}

	void SceneRenderTargets::reallocGBuffers(RenderCommandList& cmdList, bool bResolutionChanged)
	{
		if (bResolutionChanged) {
			if (gbufferA != 0) cmdList.deleteTextures(1, &gbufferA);
			if (gbufferB != 0) cmdList.deleteTextures(1, &gbufferB);
			if (gbufferC != 0) cmdList.deleteTextures(1, &gbufferC);
		}

		GLuint gbuffers[3];
		cmdList.createTextures(GL_TEXTURE_2D, 3, gbuffers);
		gbufferA = gbuffers[0];
		gbufferB = gbuffers[1];
		gbufferC = gbuffers[2];

		cmdList.textureStorage2D(gbufferA, 1, GL_RGBA32UI, sceneWidth, sceneHeight);
		cmdList.textureStorage2D(gbufferB, 1, GL_RGBA32F, sceneWidth, sceneHeight);
		cmdList.textureStorage2D(gbufferC, 1, GL_RGBA32F, sceneWidth, sceneHeight);

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
