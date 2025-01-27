#include "directional_light_component.h"
#include "pathos/console.h"

#include "badger/math/minmax.h"
#include "badger/math/vector_math.h"

namespace pathos {

	static ConsoleVariable<float> cvar_csm_zFar("r.csm.zFar", -1.0f, "Override zFar for CSM bounds. Ignored if not positive.");
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
		finalCsmCount = badger::clamp(1u, finalCsmCount, CascadedShaowMapSettings::MAX_CASCADE_COUNT);

		float finalZFar = shadowSettings.zFar;
		if (cvar_csm_zFar.getFloat() > 0.0f) finalZFar = cvar_csm_zFar.getFloat();

		proxy->directionWS = direction;
		proxy->bCastShadows = shadowSettings.bCastShadows ? 1 : 0;
		proxy->intensity = color * illuminance;
		proxy->shadowMapCascadeCount = finalCsmCount;
		proxy->directionVS = vector3(0.0f); // This is filled later
		proxy->shadowMapSize = shadowSettings.size;
		proxy->shadowMapZFar = finalZFar;
		if (proxy->bCastShadows) {
			std::vector<vector3> frustumPlanes;
			scene->camera.getFrustumVertices(frustumPlanes, finalCsmCount, finalZFar, &(proxy->csmZSlices[0]));
			for (uint32 cascadeIx = 0u; cascadeIx < finalCsmCount; ++cascadeIx) {
				calculateLightFrustumBounds(cascadeIx, &(frustumPlanes[cascadeIx * 4]), proxy);
			}
		}

		scene->proxyList_directionalLight.push_back(proxy);
	}

	void DirectionalLightComponent::calculateLightFrustumBounds(size_t cascadeIx, const vector3* frustum, DirectionalLightProxy* outProxy) {
		vector3 L_forward = direction, L_up, L_right;
		badger::calculateOrthonormalBasis(L_forward, L_up, L_right);

		vector3 center(0.0f);
		for (int32 i = 0; i < 8; ++i) center += frustum[i];
		center *= 0.125f;

		vector3 lengths(0.0f);
		for (int32 i = 0; i < 8; ++i) {
			vector3 delta = frustum[i] - center;
			lengths.x = badger::max(lengths.x, std::abs(glm::dot(delta, L_right)));
			lengths.y = badger::max(lengths.y, std::abs(glm::dot(delta, L_up)));
			lengths.z = badger::max(lengths.z, std::abs(glm::dot(delta, L_forward)));
		}

		matrix4 lightView = glm::lookAt(center, center + L_forward, L_up);
		matrix4 projection = glm::ortho(-lengths.x, lengths.x, -lengths.y, lengths.y, -lengths.z, lengths.z);

		outProxy->lightViewMatrices[cascadeIx] = lightView;
		outProxy->lightViewProjMatrices[cascadeIx] = projection * lightView;
	}

}
