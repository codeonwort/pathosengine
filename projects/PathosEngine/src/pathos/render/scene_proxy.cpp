#include "scene_proxy.h"
#include "pathos/engine_policy.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"
#include "pathos/material/material_shader.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/scene/landscape_component.h"
#include "pathos/scene/point_light_component.h"
#include "pathos/scene/rect_light_component.h"
#include "pathos/scene/directional_light_component.h"

#include "badger/math/hit_test.h"

namespace pathos {

	static constexpr uint32 RENDER_PROXY_ALLOCATOR_BYTES = 32 * 1024 * 1024; // 32 MiB

	SceneProxy::SceneProxy(const SceneProxyCreateParams& createParams)
		: sceneProxySource(createParams.proxySource)
		, frameNumber(createParams.frameNumber)
		, camera(createParams.camera)
		, fence(createParams.fence)
		, fenceValue(createParams.fenceValue)
		, lightProbeShIndex(createParams.lightProbeShIndex)
		, lightProbeColorCubemap(createParams.lightProbeColorCubemap)
		, lightProbeDepthCubemap(createParams.lightProbeDepthCubemap)
		, bSceneRenderSettingsOverriden(false)
		, renderProxyAllocator(RENDER_PROXY_ALLOCATOR_BYTES)
	{
	}

	SceneProxy::~SceneProxy() {
		proxyList_directionalLight.clear();
		proxyList_pointLight.clear();
		proxyList_rectLight.clear();
		proxyList_shadowMesh.clear();
		proxyList_staticMeshOpaque.clear();
		proxyList_staticMeshTranslucent.clear();
		proxyList_shadowMeshTrivial.clear();
		proxyList_staticMeshTrivialDepthOnly.clear();
		proxyList_landscape.clear();
		proxyList_reflectionProbe.clear();
		proxyList_irradianceVolume.clear();
		skybox = nullptr;
		cloud = nullptr;

		renderProxyAllocator.clear();
	}

	void SceneProxy::finalize_mainThread() {
		auto sortProxyList = [](StaticMeshProxyList& v) {
			std::sort(v.begin(), v.end(),
				[](const StaticMeshProxy* A, const StaticMeshProxy* B) -> bool {
					// 1. Program
					const uint32 programA = A->material->internal_getMaterialShader()->programHash;
					const uint32 programB = B->material->internal_getMaterialShader()->programHash;
					if (programA != programB) return programA < programB;
					// 2. Material instance -> wireframe -> internal -> doubleSided
					uint64 keyA = (uint64)A->material->internal_getMaterialInstanceID() << 32;
					uint64 keyB = (uint64)B->material->internal_getMaterialInstanceID() << 32;
					keyA |= ((uint64)A->material->bWireframe) << 31;
					keyA |= ((uint64)A->renderInternal) << 30;
					keyA |= ((uint64)A->doubleSided) << 29;
					keyB |= ((uint64)B->material->bWireframe) << 31;
					keyB |= ((uint64)B->renderInternal) << 30;
					keyB |= ((uint64)B->doubleSided) << 29;
					return keyA < keyB;
				}
			);
		};
		sortProxyList(proxyList_staticMeshOpaque);
		sortProxyList(proxyList_staticMeshTranslucent);
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
		// #todo-multiview: Overriding original vars, so only valid for first view.
		// Need to store WS values and shall not memcpy the proxy as is.
		for (size_t i = 0u; i < proxyList_rectLight.size(); ++i) {
			proxyList_rectLight[i]->positionVS = posToVS(proxyList_rectLight[i]->positionVS);
			proxyList_rectLight[i]->directionVS = dirToVS(proxyList_rectLight[i]->directionVS);
			proxyList_rectLight[i]->upVS = dirToVS(proxyList_rectLight[i]->upVS);
			proxyList_rectLight[i]->rightVS = dirToVS(proxyList_rectLight[i]->rightVS);
		}
		for (size_t i = 0u; i < proxyList_directionalLight.size(); ++i) {
			proxyList_directionalLight[i]->directionVS = dirToVS(proxyList_directionalLight[i]->directionWS);
		}
	}

	void SceneProxy::checkFrustumCulling(const Camera& camera) {
		Frustum3D frustum;
		camera.getFrustumPlanes(frustum);

		int32 totalCount = 0;
		int32 culledCount = 0;
		const bool bIgnoreFarPlane = (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse);

		auto checkProxyList = [&](std::vector<StaticMeshProxy*>& proxies) {
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
		};

		checkProxyList(proxyList_staticMeshOpaque);
		checkProxyList(proxyList_staticMeshTranslucent);
		// Already flagged when checking proxyList_staticMeshOpaque.
		//checkProxyList(proxyList_staticMeshTrivialDepthOnly);

		gEngine->internal_updateBasePassCullStat_renderThread(totalCount, culledCount);
	}

	void SceneProxy::addStaticMeshProxy(StaticMeshProxy* proxy) {
		if (proxy->material->internal_getMaterialShader() == nullptr) {
			return;
		}

		EMaterialShadingModel sm = proxy->material->getShadingModel();
		if (sm == EMaterialShadingModel::TRANSLUCENT) {
			proxyList_staticMeshTranslucent.push_back(proxy);
		} else {
			proxyList_staticMeshOpaque.push_back(proxy);

			MaterialShader* materialShader = proxy->material->internal_getMaterialShader();
			bool bTrivial = materialShader->bTrivialDepthOnlyPass
				&& proxy->material->bWireframe == false
				&& proxy->renderInternal == false
				&& proxy->doubleSided == false
				&& proxy->geometry->isIndex16Bit() == false
				&& proxy->geometry->isUsingPositionBufferPool();
			if (bTrivial) {
				proxy->bTrivialDepthOnly = true;
				proxyList_staticMeshTrivialDepthOnly.push_back(proxy);
			}
		}
	}

	void SceneProxy::addShadowMeshProxy(ShadowMeshProxy* proxy) {
		if (proxy->material->internal_getMaterialShader() == nullptr) {
			return;
		}

		proxyList_shadowMesh.push_back(proxy);

		MaterialShader* materialShader = proxy->material->internal_getMaterialShader();
		bool bTrivial = materialShader->bTrivialDepthOnlyPass
			&& proxy->material->bWireframe == false
			&& proxy->renderInternal == false
			&& proxy->doubleSided == false
			&& proxy->geometry->isIndex16Bit() == false
			&& proxy->geometry->isUsingPositionBufferPool();
		if (bTrivial) {
			proxy->bTrivialDepthOnly = true;
			proxyList_shadowMeshTrivial.push_back(proxy);
		}
	}

	void SceneProxy::addLandscapeProxy(LandscapeProxy* proxy) {
		proxyList_landscape.push_back(proxy);
	}

}
