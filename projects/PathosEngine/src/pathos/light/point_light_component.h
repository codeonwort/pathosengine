#pragma once

#include "pathos/actor/scene_component.h"
#include <glm/glm.hpp>

// Role model for parameters of point light:
// https://docs.unrealengine.com/en-US/Engine/Rendering/LightingAndShadows/LightTypes/Point/index.html

namespace pathos {

	struct PointLightProxy : public SceneComponentProxy {
		vector3   position;
		float     attenuationRadius;
		vector3   intensity;
		float     falloffExponent;
		uint32    castsShadow;
		vector3   padding0;
		vector4   padding1;
	};

	class PointLightComponent : public SceneComponent {
		
	public:
		virtual void createRenderProxy(Scene* scene) override
		{
			PointLightProxy* proxy = ALLOC_RENDER_PROXY<PointLightProxy>();

			proxy->position          = getLocation();
			proxy->attenuationRadius = attenuationRadius;
			proxy->intensity         = color;
			proxy->falloffExponent   = falloffExponent;
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
