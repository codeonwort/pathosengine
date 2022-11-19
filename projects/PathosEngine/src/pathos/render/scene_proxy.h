#pragma once

#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include "badger/memory/mem_alloc.h"
#include <vector>

#include "pathos/scene/scene_render_settings.h"
#include "pathos/render/render_command_list.h"
#include "pathos/material/material_id.h"
#include "pathos/scene/camera.h"

/**
 * Scene representation for the render thread.
 * Each game tick generates a SceneProxy which fully describes all resources to render a frame.
 */

namespace pathos {

	using StaticMeshProxyList = std::vector<struct StaticMeshProxy*>;

	enum class SceneProxySource : uint8 {
		MainScene    = 0,
		SceneCapture = 1,
	};

	class SceneProxy final {
		
	public:
		SceneProxy(SceneProxySource inSource, uint32 inFrameNumber, const Camera& inCamera);
		~SceneProxy();

		void finalize_mainThread();

		void overrideSceneRenderSettings(const SceneRenderSettings& inSettings);

		// This should be called for each view.
		// #todo: Parameter might be further generalized
		void createViewDependentRenderProxy(const matrix4& viewMatrix);

		void checkFrustumCulling(const Camera& camera);

		void addStaticMeshProxy(struct StaticMeshProxy* proxy);
		const StaticMeshProxyList& getOpaqueStaticMeshes() const { return proxyList_staticMeshOpaque; }
		const StaticMeshProxyList& getTranslucentStaticMeshes() const { return proxyList_staticMeshTranslucent; }

		//
		// Utilities to check if various proxies are valid.
		//
		inline bool isGodRayValid() const { return godRayMeshes.size() > 0; }
		inline bool isSkyboxValid() const { return skybox != nullptr; }
		inline bool isAnselSkyValid() const { return anselSky != nullptr; }
		inline bool isSkyAtmosphereValid() const { return skyAtmosphere != nullptr; }
		inline bool isVolumetricCloudValid() const { return cloud != nullptr; }

	public:
		SceneProxySource                           sceneProxySource;
		uint32                                     frameNumber; // number in game thread
		Camera                                     camera;

		bool                                       bScreenshotReserved = false;
		vector2i                                   screenshotSize;
		std::vector<uint16>                        screenshotRawData;

		SceneRenderSettings                        sceneRenderSettingsOverride;
		bool                                       bSceneRenderSettingsOverriden;

		// #todo-renderthread: Needs allocator pool.
		StackAllocator                             renderProxyAllocator;

		std::vector<struct DirectionalLightProxy*> proxyList_directionalLight; // first is sun
		std::vector<struct PointLightProxy*>       proxyList_pointLight;
		std::vector<struct RectLightProxy*>        proxyList_rectLight;

		// Shadowmap rendering
		std::vector<struct ShadowMeshProxy*>       proxyList_shadowMesh;
		std::vector<struct ShadowMeshProxy*>       proxyList_wireframeShadowMesh;

		std::vector<struct StaticMeshProxy*>       proxyList_staticMeshOpaque;
		std::vector<struct StaticMeshProxy*>       proxyList_staticMeshTranslucent;
		
		struct SkyboxProxy*                        skybox = nullptr;
		struct AnselSkyProxy*                      anselSky = nullptr;
		struct SkyAtmosphereProxy*                 skyAtmosphere = nullptr;
		struct VolumetricCloudProxy*               cloud = nullptr;

		// #todo-godray: Wrap with GodRayProxy class.
		//               These are filled by Scene::createRenderProxy() for now.
		std::vector<StaticMeshProxy*>              godRayMeshes;
		vector3                                    godRayLocation;

		// IBL
		GLuint                                     irradianceMap = 0;
		GLuint                                     prefilterEnvMap = 0;
		uint32                                     prefilterEnvMapMipLevels = 0;

	};

}
