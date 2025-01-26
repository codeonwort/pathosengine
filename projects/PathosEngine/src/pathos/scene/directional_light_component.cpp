#include "directional_light_component.h"
#include "pathos/console.h"

#include "badger/math/minmax.h"

namespace pathos {

	static const uint32 MAX_CASCADE_COUNT = 4u;

	// Light frustum could be too large if we use camera's zFar as is.
	static ConsoleVariable<float> cvar_csm_zFar("r.csm.zFar", -1.0f, "Custom zFar for CSM bounds");
	static ConsoleVariable<int32> cvar_csm_cascadeCount("r.csm.cascadeCount", -1, "Override shadowmap cascade count. Ignored if not positive.");

	DirectionalLightComponent::DirectionalLightComponent()
		: direction(vector3(0.0f, -1.0f, 0.0f))
		, color(vector3(1.0f, 1.0f, 1.0f))
		, illuminance(1.0f)
	{
	}

	void DirectionalLightComponent::createRenderProxy(SceneProxy* scene) {
		DirectionalLightProxy* proxy = ALLOC_RENDER_PROXY<DirectionalLightProxy>(scene);

		uint32 finalCsmCount = shadowSettings.cascadeCount;
		if (cvar_csm_cascadeCount.getInt() >= 0) finalCsmCount = cvar_csm_cascadeCount.getInt();
		finalCsmCount = badger::clamp(1u, finalCsmCount, MAX_CASCADE_COUNT);

		float finalZFar = shadowSettings.zFar;
		if (cvar_csm_zFar.getFloat() > 0.0f) finalZFar = cvar_csm_zFar.getFloat();

		proxy->directionWS = direction;
		proxy->bCastShadows = shadowSettings.bCastShadows ? 1 : 0;
		proxy->intensity = color * illuminance;
		proxy->shadowMapCascadeCount = finalCsmCount;
		proxy->directionVS = vector3(0.0f); // This is filled later
		proxy->shadowMapSize = shadowSettings.size;
		proxy->shadowMapZFar = finalZFar;

		scene->proxyList_directionalLight.push_back(proxy);
	}

}
