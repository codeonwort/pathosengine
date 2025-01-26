#pragma once

#include "pathos/rhi/render_command_list.h"
#include "pathos/render/scene_proxy.h"

namespace pathos {

	class Texture;
	struct DirectionalLightProxy;

	constexpr uint32 SKY_IRRADIANCE_MAP_SIZE = 32;

	// For sky atmosphere and panorama sky. Skybox will use the size of its source cubemap.
	constexpr uint32 SKY_PREFILTER_MAP_DEFAULT_SIZE = 512;
	constexpr uint32 SKY_PREFILTER_MAP_MIN_SIZE = 128;
	constexpr uint32 SKY_PREFILTER_MAP_MAX_NUM_MIPS = 5;
	
	// Textures for scene rendering
	// #todo-renderer: render target pool for temporary textures
	struct SceneRenderTargets {

	private:
		bool bDestroyed = false;

	public:
		// CAUTION: These can be different than those of SceneRenderSettings due to resolution scaling.
		// Scene size before super resolution.
		// Both super resolution and screen percentage are applied.
		uint32 sceneWidth = 0;
		uint32 sceneHeight = 0;

		// Scene size after super resolution. Screen percentage is still applied.
		uint32 sceneWidthSuperRes = 0;
		uint32 sceneHeightSuperRes = 0;

		// Scene size after screen percentage.
		// Same as width/height values in SceneRenderSettings.
		uint32 unscaledSceneWidth = 0;
		uint32 unscaledSceneHeight = 0;

		GLuint sceneColor = 0;        // Accumulate scene luminance across multiple passes, before any post-processing.
		GLuint sceneColorHalfRes = 0; // Half res version, generated before any post processing starts.
		                              // PP passes that are too expensive to run in full res should use this.
		GLuint sceneDepth = 0;        // Written in depth pre-pass.

		// Auto exposure (average log luminance)
		GLuint sceneLuminance = 0;
		uint32 sceneLuminanceSize = 0;
		uint32 sceneLuminanceMipCount = 0;

		// Auto exposure (histogram)
		GLuint luminanceFromHistogram = 0;

		GLuint sceneColorAA = 0; // sceneColor after anti-aliasing pass
		GLuint sceneColorHistory = 0; // For TAA
		GLuint velocityMap = 0;

		GLuint sceneColorUpscaledTemp = 0; // FSR1 EASU output
		GLuint sceneColorUpscaled = 0; // FSR1 RCAS output

		GLuint sceneFinal = 0; // Final texture rendered on the screen

		// Screen Space Reflection
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

		// Volumetric Clouds
		GLuint volumetricCloudA = 0; // Prev and current, rotated
		GLuint volumetricCloudB = 0; // Prev and current, rotated

		// Cascaded Shadow Maps
	private:
		uint32 cachedCsmCount = 0;
		uint32 cachedCsmSize = 0;
	public:
		GLuint cascadedShadowMap = 0;

		// Omnidirectional Shadow Maps
		uint32 omniShadowMapLayerCount = 0;
		uint32 omniShadowMapSize = 0;
		GLuint omniShadowMaps = 0; // cubemap array

		// Indirect Lighting
		GLuint localSpecularIBLs = 0;        // Cubemap array for local reflection probes
		GLuint skyIrradianceMap = 0;         // Cubemap for sky indirect diffuse
		GLuint skyPrefilteredMap = 0;        // Cubemap for sky indirect specular
		uint32 skyPrefilterMapMipCount = 1;
		uint32 skyPrefilterMapSize = 0;

		// Deferred Shading
		bool useGBuffer = true;
		GLuint gbufferA = 0;
		GLuint gbufferB = 0;
		GLuint gbufferC = 0;

		// Post Processing: God Ray
		GLuint godRaySource = 0;
		GLuint godRayResult = 0;
		GLuint godRayResultTemp = 0;

		// Post Processing: Depth of Field
		GLuint sceneColorDoFInput = 0;
		GLuint dofSubsum0 = 0;
		GLuint dofSubsum1 = 0;

		// Post Processing: Bloom
		GLuint sceneBloomSetup = 0;         // Source for sceneBloomChain mip0
		uint32 sceneBloomChainMipCount = 0; // # of mipmaps of sceneBloomChain
		GLuint sceneBloomChain = 0;         // mip0: half resolution, mip1: quarter resolution, mip2: ...
		std::vector<GLuint> sceneBloomChainViews;

		// Post Processing: Tone Mapping
		GLuint sceneColorToneMapped = 0;

		// Post Processing: Screen Space Ambient Occlusion
		GLuint ssaoHalfNormalAndDepth = 0;
		GLuint ssaoMap = 0;
		GLuint ssaoMapTemp = 0;

	public:
		SceneRenderTargets();
		~SceneRenderTargets();

		SceneRenderTargets(const SceneRenderTargets&) = delete;
		SceneRenderTargets& operator=(const SceneRenderTargets&) = delete;

		// Reallocate scene textures if they are invalid or the screen resolution has been changed.
		// 
		// @param sceneProxySource         : For light-weight sources, some render targets are not created.
		// @param bEnableResolutionScaling : Texture sizes will be affected by screen percentage and/or super resolution.
		void reallocSceneTextures(
			RenderCommandList& cmdList,
			SceneProxySource sceneProxySource,
			uint32 width,
			uint32 height,
			bool bEnableResolutionScaling);

		void freeSceneTextures(RenderCommandList& cmdList);

		// If lightProxy is null, then deallocate relevant resources.
		void reallocDirectionalShadowMaps(RenderCommandList& cmdList, const DirectionalLightProxy* lightProxy);

		void reallocOmniShadowMaps(RenderCommandList& cmdList, uint32 numPointLights, uint32 shadowMapSize);
		void reallocGBuffers(RenderCommandList& cmdList, bool bResolutionChanged);
		void reallocSkyIrradianceMap(RenderCommandList& cmdList);
		void reallocSkyPrefilterMap(RenderCommandList& cmdList, uint32 cubemapSize);

		GLuint getSkyIrradianceMapWithFallback() const;
		GLuint getSkyPrefilterMapWithFallback() const;
		uint32 getSkyPrefilterMapMipCount() const;

		GLuint getVolumetricCloud(uint32 frameCounter) const {
			return (frameCounter % 2 == 0) ? volumetricCloudA : volumetricCloudB;
		}
		GLuint getPrevVolumetricCloud(uint32 frameCounter) const {
			return (frameCounter % 2 == 0) ? volumetricCloudB : volumetricCloudA;
		}

	};

}
