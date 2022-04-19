#include "scene_proxy.h"
#include "pathos/light/point_light_component.h"
#include "pathos/light/directional_light_component.h"

namespace pathos
{
	static constexpr uint32 RENDER_PROXY_ALLOCATOR_BYTES = 32 * 1024 * 1024; // 32 MB

	SceneProxy::SceneProxy(SceneProxySource inSource, uint32 inFrameNumber, const Camera& inCamera)
		: sceneProxySource(inSource)
		, frameNumber(inFrameNumber)
		, camera(inCamera)
		, bSceneRenderSettingsOverriden(false)
		, renderProxyAllocator(RENDER_PROXY_ALLOCATOR_BYTES)
	{
		//
	}

	SceneProxy::~SceneProxy() {
		proxyList_directionalLight.clear();
		proxyList_pointLight.clear();
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
		for (uint32 i = 0u; i < proxyList_pointLight.size(); ++i) {
			proxyList_pointLight[i]->viewPosition = vector3(viewMatrix * vector4(proxyList_pointLight[i]->worldPosition, 1.0f));
		}

		for (uint32 i = 0u; i < proxyList_directionalLight.size(); ++i) {
			proxyList_directionalLight[i]->vsDirection = vector3(viewMatrix * vector4(proxyList_directionalLight[i]->wsDirection, 0.0f));
		}
	}

}
