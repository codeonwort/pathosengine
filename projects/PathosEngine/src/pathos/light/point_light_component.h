#pragma once

#include "pathos/actor/scene_component.h"

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
	};

	class PointLightComponent : public SceneComponent {
		
	public:
		virtual void createRenderProxy(Scene* scene) override
		{
			PointLightProxy* proxy = ALLOC_RENDER_PROXY<PointLightProxy>();

			proxy->worldPosition     = getLocation();
			proxy->attenuationRadius = attenuationRadius;
			proxy->intensity         = color;
			proxy->falloffExponent   = falloffExponent;
			proxy->viewPosition      = vector3(0.0f); // NOTE: This is filled later
			proxy->castsShadow       = castsShadow;

			scene->proxyList_pointLight.push_back(proxy);
		}

	public:
		vector3   color;
		float     attenuationRadius;
		float     falloffExponent;
		bool      castsShadow;

	};

}
