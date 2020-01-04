#pragma once

#include "render_command_list.h"

namespace pathos {
	
	// #todo-scene-textures: Use this struct in forward renderer
	// Textures for scene rendering
	struct SceneRenderTargets {

	private:
		bool destroyed = false;

	public:
		uint32 sceneWidth;
		uint32 sceneHeight;

		uint32 numCascades = 4;
		uint32 csmWidth = 2048;
		uint32 csmHeight = 2048;

		GLuint sceneFinal; // Final texture rendered on the screen
		GLuint sceneColor; // This usually end up as an unpack of gbuffer, before any post-processing
		GLuint sceneDepth;
		GLuint cascadedShadowMap;

		// Deferred renderer only
		bool useGBuffer;
		GLuint gbufferA;
		GLuint gbufferB;
		GLuint gbufferC;

		// post processing: god ray
		GLuint godRaySource;
		GLuint godRayResult;
		GLuint godRayResultFiltered;

		// post processing: depth of field
		GLuint dofSubsum0;
		GLuint dofSubsum1;

		// post processing: bloom
		GLuint sceneBloom;
		GLuint sceneBloomTemp;

		// post processing: tone mapping
		GLuint toneMappingResult;

		// post processing: ssao
		GLuint ssaoHalfNormalAndDepth;
		GLuint ssaoMap;
		GLuint ssaoMapTemp;

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
