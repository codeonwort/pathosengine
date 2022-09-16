#include "scene_proxy.h"
#include "pathos/engine_policy.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/light/point_light_component.h"
#include "pathos/light/rect_light_component.h"
#include "pathos/light/directional_light_component.h"

#include "badger/math/hit_test.h"

namespace pathos {

	static constexpr uint32 RENDER_PROXY_ALLOCATOR_BYTES = 32 * 1024 * 1024; // 32 MB

	SceneProxy::SceneProxy(SceneProxySource inSource, uint32 inFrameNumber, const Camera& inCamera)
		: sceneProxySource(inSource)
		, frameNumber(inFrameNumber)
		, camera(inCamera)
		, bSceneRenderSettingsOverriden(false)
		, renderProxyAllocator(RENDER_PROXY_ALLOCATOR_BYTES)
	{
	}

	SceneProxy::~SceneProxy() {
		proxyList_directionalLight.clear();
		proxyList_pointLight.clear();
		proxyList_rectLight.clear();
		proxyList_shadowMesh.clear();
		proxyList_wireframeShadowMesh.clear();
		for (uint32 i = 0; i < (uint32)MATERIAL_ID::NUM_MATERIAL_IDS; ++i) {
			proxyList_staticMesh[i].clear();
		}
		skybox = nullptr;
		cloud = nullptr;

		renderProxyAllocator.clear();
	}

	void SceneProxy::overrideSceneRenderSettings(const SceneRenderSettings& inSettings) {
		sceneRenderSettingsOverride = inSettings;
		bSceneRenderSettingsOverriden = true;
	}

	void SceneProxy::createViewDependentRenderProxy(const matrix4& viewMatrix) {
		auto posToVS = [&viewMatrix](const vector3& posWS) {
			return vector3(viewMatrix * vector4(posWS, 1.0f));
		};
		auto dirToVS = [&viewMatrix](const vector3& dirWS) {
			return vector3(viewMatrix * vector4(dirWS, 0.0f));
		};

		for (size_t i = 0u; i < proxyList_pointLight.size(); ++i) {
			proxyList_pointLight[i]->viewPosition = posToVS(proxyList_pointLight[i]->worldPosition);
		}
		for (size_t i = 0u; i < proxyList_rectLight.size(); ++i) {
			proxyList_rectLight[i]->positionVS = posToVS(proxyList_rectLight[i]->positionWS);
			proxyList_rectLight[i]->directionVS = dirToVS(proxyList_rectLight[i]->directionWS);
		}
		for (size_t i = 0u; i < proxyList_directionalLight.size(); ++i) {
			proxyList_directionalLight[i]->vsDirection = dirToVS(proxyList_directionalLight[i]->wsDirection);
		}
	}

	void SceneProxy::checkFrustumCulling(const Camera& camera) {
		Frustum3D frustum;
		camera.getFrustumPlanes(frustum);

		// #todo-frustum-culling: Stat command for culling status
		int32 totalCount = 0;
		int32 culledCount = 0;
		const bool bIgnoreFarPlane = (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse);

		const uint8 numMaterialIDs = (uint8)MATERIAL_ID::NUM_MATERIAL_IDS;
		for (uint8 materialID = 0; materialID < numMaterialIDs; ++materialID) {
			auto& proxies = proxyList_staticMesh[materialID];
			for (int32 i = 0; i < proxies.size(); ++i) {
				if (bIgnoreFarPlane) {
					proxies[i]->bInFrustum = badger::hitTest::AABB_frustum_noFarPlane(proxies[i]->worldBounds, frustum);
				} else {
					proxies[i]->bInFrustum = badger::hitTest::AABB_frustum(proxies[i]->worldBounds, frustum);
				}

				if (!proxies[i]->bInFrustum) {
					culledCount++;
				}
				totalCount++;
			}
		}
	}

}
