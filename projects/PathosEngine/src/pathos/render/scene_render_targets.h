#pragma once

#include "render_command_list.h"

namespace pathos {
	
	// Textures for scene rendering
	// #todo-renderer: render target pool for temporary textures
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

		GLuint sceneColorDownsampleChain; // mip0: half resolution, mip1: quarter resolution, mip2: ...
		uint32 sceneColorDownsampleMipmapCount; // # of mipmaps of sceneColorDownsampleChain
		std::vector<GLuint> sceneColorDownsampleViews;

		// Screen space reflection
		GLuint sceneDepthHiZ = 0;
		uint32 sceneDepthHiZMipmapCount = 0;
		std::vector<GLuint> sceneDepthHiZViews;
		GLuint ssrPreintegration = 0;
		uint32 ssrPreintegrationMipmapCount = 0;
		std::vector<GLuint> ssrPreintegrationViews;

		GLuint volumetricCloudA; // Prev and current, rotated
		GLuint volumetricCloudB; // Prev and current, rotated

		GLuint cascadedShadowMap;
		GLuint omniShadowMaps; // cubemap array

		// Deferred renderer only
		bool useGBuffer;
		GLuint gbufferA;
		GLuint gbufferB;
		GLuint gbufferC;

		// post processing: god ray
		GLuint godRaySource;
		GLuint godRayResult;
		GLuint godRayResultTemp;

		// post processing: depth of field
		GLuint dofSubsum0;
		GLuint dofSubsum1;

		// post processing: bloom
		GLuint sceneBloom;
		GLuint sceneBloomTemp;
		std::vector<GLuint> sceneBloomViews;
		std::vector<GLuint> sceneBloomTempViews;

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

		// Reallocate scene textures if they are invalid or the screen resolution has been changed
		void reallocSceneTextures(RenderCommandList& cmdList, uint32 width, uint32 height);

		void freeSceneTextures(RenderCommandList& cmdList);

		// Called every frame by renderer
		void reallocOmniShadowMaps(RenderCommandList& cmdList, uint32 numPointLights, uint32 width, uint32 height);

		// Deferred renderer only
		void reallocGBuffers(RenderCommandList& cmdList, bool bResolutionChanged);

		GLuint getVolumetricCloud(uint32 frameCounter) const {
			return (frameCounter % 2 == 0) ? volumetricCloudA : volumetricCloudB;
		}
		GLuint getPrevVolumetricCloud(uint32 frameCounter) const {
			return (frameCounter % 2 == 0) ? volumetricCloudB : volumetricCloudA;
		}

	};

}
