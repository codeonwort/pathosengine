#pragma once

#include "pathos/actor/actor.h"
#include "pathos/material/material_id.h"

#include "badger/types/matrix_types.h"

#include "gl_core.h"
#include <vector>

namespace pathos {

	// Forward declaration
	class StaticMeshComponent;
	class SkyRendering;
	class VolumetricCloudActor;

	// Represents a 3D scene.
	class Scene {
		friend class World;

	public:
		Scene()                        = default;
		Scene(const Scene&)            = delete;
		Scene& operator=(const Scene&) = delete;

		//////////////////////////////////////////////////////////////////////////
		// New API

		void clearRenderProxy();
		void createRenderProxy();

		World* getWorld() const { return owner; }

		//////////////////////////////////////////////////////////////////////////
		// Old API

		void transformLightProxyToViewSpace(const matrix4& viewMatrix);

	public:
		SkyRendering* sky = nullptr;
		StaticMeshComponent* godRaySource = nullptr;
		VolumetricCloudActor* cloud = nullptr;

		// IBL
		GLuint irradianceMap = 0;
		GLuint prefilterEnvMap = 0;
		uint32 prefilterEnvMapMipLevels = 0;

	public:
		std::vector<struct DirectionalLightProxy*> proxyList_directionalLight; // first is sun
		std::vector<struct PointLightProxy*>       proxyList_pointLight;
		std::vector<struct ShadowMeshProxy*>       proxyList_shadowMesh;
		std::vector<struct ShadowMeshProxy*>       proxyList_wireframeShadowMesh;
		std::vector<struct StaticMeshProxy*>       proxyList_staticMesh[(uint32)MATERIAL_ID::NUM_MATERIAL_IDS];

	protected:
		World* owner = nullptr;

	};

}
