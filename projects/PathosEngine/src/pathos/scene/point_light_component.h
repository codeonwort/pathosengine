#pragma once

#include "pathos/scene/scene_component.h"

// Role model for parameters of point light:
// https://docs.unrealengine.com/en-US/Engine/Rendering/LightingAndShadows/LightTypes/Point/index.html

namespace pathos {

	struct PointLightProxy : public SceneComponentProxy {
		vector3   worldPosition;
		float     attenuationRadius;

		vector3   intensity; // Multiplied by color
		float     falloffExponent;

		vector3   viewPosition;
		uint32    castsShadow;

		float     sourceRadius;
		vector3   padding0;
	};

	class PointLightComponent : public SceneComponent {
		
	public:
		PointLightComponent()
			: color(vector3(1.0f, 1.0f, 1.0f))
			, intensity(10.0f)
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
			proxy->intensity         = color * intensity;
			proxy->falloffExponent   = falloffExponent;
			proxy->viewPosition      = vector3(0.0f); // NOTE: This is filled later
			proxy->castsShadow       = castsShadow;
			proxy->sourceRadius      = (std::max)(0.01f, sourceRadius);

			scene->proxyList_pointLight.push_back(proxy);
		}

	public:
		vector3   color;             // Luminous efficiency function. Should be clamped to [0, 1]
		float     intensity;         // Unit: candela = lm/sr = luminuous intensity
		float     attenuationRadius; // Unit: meter
		float     falloffExponent;
		bool      castsShadow;
		float     sourceRadius;      // Unit: meter

	};

}
