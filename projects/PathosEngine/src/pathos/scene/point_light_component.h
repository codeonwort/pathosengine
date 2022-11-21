#pragma once

#include "pathos/scene/scene_component.h"

// Role model for parameters of point light:
// https://docs.unrealengine.com/en-US/Engine/Rendering/LightingAndShadows/LightTypes/Point/index.html

namespace pathos {

	struct PointLightProxy : public SceneComponentProxy {
		vector3   worldPosition;
		float     attenuationRadius;
		vector3   intensity;
		float     falloffExponent;
		vector3   viewPosition;
		uint32    castsShadow;
		float     sourceRadius;
		vector3   padding0;
	};

	class PointLightComponent : public SceneComponent {
		
	public:
		PointLightComponent()
			: intensity(vector3(100.0f, 100.0f, 100.0f))
			, attenuationRadius(1.0f)
			, falloffExponent(0.001f)
			, castsShadow(true)
			, sourceRadius(0.01f)
		{
		}

		virtual void createRenderProxy(SceneProxy* scene) override
		{
			PointLightProxy* proxy = ALLOC_RENDER_PROXY<PointLightProxy>(scene);

			proxy->worldPosition     = getLocation();
			proxy->attenuationRadius = attenuationRadius;
			proxy->intensity         = intensity;
			proxy->falloffExponent   = falloffExponent;
			proxy->viewPosition      = vector3(0.0f); // NOTE: This is filled later
			proxy->castsShadow       = castsShadow;
			proxy->sourceRadius      = (std::max)(0.01f, sourceRadius);

			scene->proxyList_pointLight.push_back(proxy);
		}

	public:
		vector3   intensity;         // #todo-measurement: Photometric unit (maybe candela)
		float     attenuationRadius; // in meters
		float     falloffExponent;
		bool      castsShadow;
		float     sourceRadius;      // in meters

	};

}
