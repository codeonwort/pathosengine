#pragma once

#include "render_command_list.h"

namespace pathos {
	
	// #todo-scene-textures: Use this struct in forward/deferred renderers
	// Textures for scene rendering
	struct SceneRenderTargets {

	public:
		uint32 sceneWidth;
		uint32 sceneHeight;

		GLuint sceneColor;
		GLuint sceneDepth;
		GLuint cascadedShadowMap;

		// Deferred renderer only
		bool useGBuffer;
		GLuint gbufferA;
		GLuint gbufferB;
		GLuint gbufferC;

	public:
		SceneRenderTargets();
		~SceneRenderTargets();

		SceneRenderTargets(const SceneRenderTargets&) = delete;
		SceneRenderTargets& operator=(const SceneRenderTargets&) = delete;

		// Reallocate scene textures if invalid or screen resolution changed
		void reallocSceneTextures(RenderCommandList& cmdList, uint32 width, uint32 height);

		void freeSceneTextures(RenderCommandList& cmdList);

		// Deferred renderer only
		void reallocGBuffers(RenderCommandList& cmdList, bool bResolutionChanged);

	};

}
