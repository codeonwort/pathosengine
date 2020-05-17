#pragma once

#include "pathos/actor/scene_component.h"
#include <glm/glm.hpp>

// Role model for parameters of point light:
// https://docs.unrealengine.com/en-US/Engine/Rendering/LightingAndShadows/LightTypes/Point/index.html

namespace pathos {

	struct PointLightProxy : public SceneComponentProxy {
		glm::vec3 position;
		float     attenuationRadius;
		glm::vec3 intensity;
		float     falloffExponent;
	};

	class PointLightComponent : public SceneComponent {
		
	public:
		virtual void createRenderProxy(Scene* scene) override
		{
			PointLightProxy* proxy = ALLOC_RENDER_PROXY<PointLightProxy>();

			proxy->position          = position;
			proxy->attenuationRadius = attenuationRadius;
			proxy->intensity         = color;
			proxy->falloffExponent   = falloffExponent;

			scene->proxyList_pointLight.push_back(proxy);
		}

	public:
		glm::vec3 position;
		glm::vec3 color;
		float     attenuationRadius;
		float     falloffExponent;

	};

}
