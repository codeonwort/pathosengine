#pragma once

#include "pathos/actor/scene_component.h"
#include <glm/glm.hpp>

namespace pathos {

	struct DirectionalLightProxy : public SceneComponentProxy {
		glm::vec3 direction;
		float     padding0;
		glm::vec3 radiance;
		float     padding1;
	};

	class DirectionalLightComponent : public SceneComponent {

	public:
		DirectionalLightComponent()
			: direction(glm::vec3(0.0f, -1.0f, 0.0f))
			, radiance(glm::vec3(1.0f, 1.0f, 1.0f))
		{}

		virtual void createRenderProxy(Scene* scene) override {
			DirectionalLightProxy* proxy = ALLOC_RENDER_PROXY<DirectionalLightProxy>();

			proxy->direction = direction;
			proxy->padding0  = 0.0f;
			proxy->radiance  = radiance;
			proxy->padding1  = 0.0f;

			scene->proxyList_directionalLight.push_back(proxy);
		}

	public:
		glm::vec3 direction;
		glm::vec3 radiance;

	};

}
