#include "scene_render_targets.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	SceneRenderTargets::SceneRenderTargets()
		: sceneWidth(0)
		, sceneHeight(0)
		, sceneColor(0)
		, sceneDepth(0)
		, cascadedShadowMap(0)
		, useGBuffer(false)
		, gbufferA(0)
		, gbufferB(0)
		, gbufferC(0)
	{
	}

	SceneRenderTargets::~SceneRenderTargets()
	{
		CHECK(sceneColor == 0);
		CHECK(sceneDepth == 0);
		CHECK(cascadedShadowMap == 0);
		CHECK(gbufferA == 0);
		CHECK(gbufferB == 0);
		CHECK(gbufferC == 0);
	}

	void SceneRenderTargets::reallocSceneTextures(RenderCommandList& cmdList, uint32 width, uint32 height)
	{
		CHECK(width > 0 && height > 0);

		const bool bResolutionChanged = sceneWidth != width || sceneHeight != height;
		sceneWidth = width;
		sceneHeight = height;

		// sceneColor
		if (sceneColor == 0 || bResolutionChanged) {
			if (sceneColor != 0) {
				cmdList.deleteTextures(1, &sceneColor);
			}
			cmdList.createTextures(GL_TEXTURE_2D, 1, &sceneColor);
			cmdList.textureStorage2D(sceneColor, 1, GL_RGBA32F, width, height);
		}

		// sceneDepth
		if (sceneDepth == 0 || bResolutionChanged) {
			if (sceneDepth != 0) {
				cmdList.deleteTextures(1, &sceneDepth);
			}
			cmdList.createTextures(GL_TEXTURE_2D, 1, &sceneDepth);
			cmdList.textureStorage2D(sceneDepth, 1, GL_DEPTH24_STENCIL8, width, height);
		}

		// #todo-scene-textures: alloc CSM

		if (useGBuffer) {
			reallocGBuffers(cmdList, bResolutionChanged);
		}
	}

	void SceneRenderTargets::freeSceneTextures(RenderCommandList& cmdList)
	{
#define safe_release(x) if(x != 0) { cmdList.deleteTextures(1, &x); x = 0; }
		safe_release(sceneColor);
		safe_release(sceneDepth);
		safe_release(cascadedShadowMap);
		safe_release(gbufferA);
		safe_release(gbufferB);
		safe_release(gbufferC);
#undef safe_release
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

		cmdList.textureStorage2D(gbufferA, 1, GL_RGBA32F, sceneWidth, sceneHeight);
		cmdList.textureStorage2D(gbufferB, 1, GL_RGBA32F, sceneWidth, sceneHeight);
		cmdList.textureStorage2D(gbufferC, 1, GL_RGBA32F, sceneWidth, sceneHeight);
	}

}
