#pragma once

#include "pathos/render/scene_proxy_common.h"
#include "pathos/rhi/render_command_list.h"
#include "pathos/material/material_id.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/scene_render_settings.h"

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include "badger/system/mem_alloc.h"
#include <vector>

/**
 * Scene representation for the render thread.
 * Each game tick generates a SceneProxy which fully describes all resources to render a frame.
 */

namespace pathos {

	class Fence;
	class Buffer;
	class DirectionalLightComponent;

	using DirectionalLightProxyList = std::vector<struct DirectionalLightProxy*>;
	using PointLightProxyList       = std::vector<struct PointLightProxy*>;
	using RectLightProxyList        = std::vector<struct RectLightProxy*>;
	using ShadowMeshProxyList       = std::vector<struct ShadowMeshProxy*>;
	using StaticMeshProxyList       = std::vector<struct StaticMeshProxy*>;
	using LandscapeProxyList        = std::vector<struct LandscapeProxy*>;
	using ReflectionProbeProxyList  = std::vector<struct ReflectionProbeProxy*>;
	using IrradianceVolumeProxyList = std::vector<struct IrradianceVolumeProxy*>;

	class SceneProxy final {
		
	public:
		SceneProxy(const SceneProxyCreateParams& createParams);
		~SceneProxy();

		void finalize_mainThread();

		// DO NOT USE. Dirty hack :(
		inline void internal_setSunComponent(DirectionalLightComponent* dirLightComponent) { tempSunComponent = dirLightComponent; }
		inline DirectionalLightComponent* internal_getSunComponent() { return tempSunComponent; }
		inline Fence* internal_getFence() const { return fence; }
		inline uint64 internal_getFenceValue() const { return fenceValue; }

		void overrideSceneRenderSettings(const SceneRenderSettings& inSettings);

		// This should be called for each view.
		// #todo-scene-proxy: Parameter might be further generalized
		void createViewDependentRenderProxy(const matrix4& viewMatrix);

		void checkFrustumCulling(const Camera& camera);

		void addStaticMeshProxy(struct StaticMeshProxy* proxy);
		const StaticMeshProxyList& getOpaqueStaticMeshes() const { return proxyList_staticMeshOpaque; }
		const StaticMeshProxyList& getTranslucentStaticMeshes() const { return proxyList_staticMeshTranslucent; }
		const StaticMeshProxyList& getTrivialDepthOnlyStaticMeshes() const { return proxyList_staticMeshTrivialDepthOnly; }

		void addShadowMeshProxy(struct ShadowMeshProxy* proxy);
		const ShadowMeshProxyList& getShadowMeshes() const { return proxyList_shadowMesh; }
		const ShadowMeshProxyList& getTrivialShadowMeshes() const { return proxyList_shadowMeshTrivial; }

		void addLandscapeProxy(struct LandscapeProxy* proxy);
		const LandscapeProxyList& getLandscapeMeshes() const { return proxyList_landscape; }

		//
		// Utilities to check if various proxies are valid.
		//
		inline bool isGodRayValid() const { return godRayMeshes.size() > 0; }
		inline bool isSkyboxValid() const { return skybox != nullptr; }
		inline bool isPanoramaSkyValid() const { return panoramaSky != nullptr; }
		inline bool isSkyAtmosphereValid() const { return skyAtmosphere != nullptr; }
		inline bool isVolumetricCloudValid() const { return cloud != nullptr; }

	public:
		SceneProxySource                           sceneProxySource;
		uint32                                     frameNumber; // number in game thread
		Camera                                     camera;

		// For sceneProxySource == IrradianceCapture
		uint32                                     lightProbeShIndex = IrradianceProbeAtlasDesc::INVALID_TILE_ID;
		Texture*                                   lightProbeColorCubemap = nullptr;
		Texture*                                   lightProbeDepthCubemap = nullptr;
		vector4ui                                  lightProbeDepthAtlasCoordAndSize = vector4ui(0);

		float                                      deltaSeconds = 0.0f;

		bool                                       bScreenshotReserved = false;
		vector2i                                   screenshotSize = vector2i(0, 0);
		std::vector<uint16>                        screenshotRawData;

		SceneRenderSettings                        sceneRenderSettingsOverride;
		bool                                       bSceneRenderSettingsOverriden;

		// #todo-renderthread: Needs allocator pool.
		StackAllocator                             renderProxyAllocator;

		DirectionalLightProxyList                  proxyList_directionalLight; // first is sun
		PointLightProxyList                        proxyList_pointLight;
		RectLightProxyList                         proxyList_rectLight;

		// Shadowmap rendering
		ShadowMeshProxyList                        proxyList_shadowMesh;

		// Standard mesh proxies
		StaticMeshProxyList                        proxyList_staticMeshOpaque;
		StaticMeshProxyList                        proxyList_staticMeshTranslucent;

		// #todo-indirect-draw: Ultimately wanna merge every drawcall groups but let's start with the easiest one.
		// Subset of proxyList_shadowMesh whose drawcalls can be merged.
		ShadowMeshProxyList                        proxyList_shadowMeshTrivial;
		// Subset of proxyList_staticMeshOpaque whose depth-only drawcalls can be merged.
		StaticMeshProxyList                        proxyList_staticMeshTrivialDepthOnly;

		// Landscape
		LandscapeProxyList                         proxyList_landscape;
		
		bool                                       bInvalidateSkyLighting = false;
		struct SkyboxProxy*                        skybox = nullptr;
		struct PanoramaSkyProxy*                   panoramaSky = nullptr;
		struct SkyAtmosphereProxy*                 skyAtmosphere = nullptr;
		struct VolumetricCloudProxy*               cloud = nullptr;

		// #todo-godray: Wrap with GodRayProxy class. These are filled by Scene::createRenderProxy() for now.
		StaticMeshProxyList                        godRayMeshes;
		vector3                                    godRayLocation = vector3(0.0f);
		vector3                                    godRayColor = vector3(1.0f, 0.5f, 0.0f);
		float                                      godRayIntensity = 1.0f;

		// Light probe
		float                                      irradianceAtlasWidth = 0.0f;
		float                                      irradianceAtlasHeight = 0.0f;
		uint32                                     irradianceTileCountX = 0;
		uint32                                     irradianceTileSize = 0;
		IrradianceVolumeProxyList                  proxyList_irradianceVolume;
		GLuint                                     irradianceAtlas = 0;
		GLuint                                     depthProbeAtlas = 0;
		GLuint                                     irradianceVolumeBuffer = 0;
		GLuint                                     reflectionProbeBuffer = 0;
		Buffer*                                    irradianceSHBuffer = nullptr;

		ReflectionProbeProxyList                   proxyList_reflectionProbe;
		Texture*                                   reflectionProbeArrayTexture = nullptr;

	private:
		DirectionalLightComponent*                 tempSunComponent = nullptr;

		Fence*                                     fence;
		uint64                                     fenceValue;
	};

}
