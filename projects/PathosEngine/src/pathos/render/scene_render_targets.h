#pragma once

#include "render_command_list.h"

namespace pathos {
	
	// Textures for scene rendering
	// #todo-renderer: render target pool for temporary textures
	struct SceneRenderTargets {

	private:
		bool destroyed = false;

	public:
		uint32 sceneWidth = 0;
		uint32 sceneHeight = 0;

		uint32 numCascades = 4;
		uint32 csmWidth = 2048;
		uint32 csmHeight = 2048;

		GLuint sceneFinal = 0; // Final texture rendered on the screen
		GLuint sceneColor = 0; // This usually end up as an unpack of gbuffer, before any post-processing
		GLuint sceneDepth = 0;

		GLuint sceneColorDownsampleChain = 0; // mip0: half resolution, mip1: quarter resolution, mip2: ...
		uint32 sceneColorDownsampleMipmapCount = 0; // # of mipmaps of sceneColorDownsampleChain
		std::vector<GLuint> sceneColorDownsampleViews;

		// Screen space reflection
		GLuint sceneDepthHiZ = 0;
		uint32 sceneDepthHiZMipmapCount = 0;
		std::vector<GLuint> sceneDepthHiZViews;
		GLuint ssrPreintegration = 0;
		uint32 ssrPreintegrationMipmapCount = 0;
		std::vector<GLuint> ssrPreintegrationViews;
		GLuint ssrRayTracing = 0;
		GLuint ssrPreconvolution = 0;
		GLuint ssrPreconvolutionTemp = 0;
		uint32 ssrPreconvolutionMipmapCount = 0;
		std::vector<GLuint> ssrPreconvolutionViews;
		std::vector<GLuint> ssrPreconvolutionTempViews;

		GLuint volumetricCloudA = 0; // Prev and current, rotated
		GLuint volumetricCloudB = 0; // Prev and current, rotated

		GLuint cascadedShadowMap = 0;
		GLuint omniShadowMaps = 0; // cubemap array

		// Deferred renderer only
		bool useGBuffer = false;
		GLuint gbufferA = 0;
		GLuint gbufferB = 0;
		GLuint gbufferC = 0;

		// post processing: god ray
		GLuint godRaySource = 0;
		GLuint godRayResult = 0;
		GLuint godRayResultTemp = 0;

		// post processing: depth of field
		GLuint dofSubsum0 = 0;
		GLuint dofSubsum1 = 0;

		// post processing: bloom
		GLuint sceneBloom = 0;
		GLuint sceneBloomTemp = 0;
		std::vector<GLuint> sceneBloomViews;
		std::vector<GLuint> sceneBloomTempViews;

		// post processing: tone mapping
		GLuint toneMappingResult = 0;

		// post processing: ssao
		GLuint ssaoHalfNormalAndDepth = 0;
		GLuint ssaoMap = 0;
		GLuint ssaoMapTemp = 0;

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
