#pragma once

#include "pathos/actor/scene_component.h"

namespace pathos {

	struct RectLightProxy : public SceneComponentProxy {
		vector3   positionWS;
		float     attenuationRadius;

		vector3   directionWS;
		uint32    castsShadow;

		vector3   intensity;
		float     falloffExponent;

		float     width;
		float     height;
		vector2   _padding0;

		vector3   positionVS;
		float     _padding1;

		vector3   directionVS;
		float     _padding2;
	};

	class RectLightComponent : public SceneComponent {

	public:
		RectLightComponent()
			: intensity(vector3(1.0f))
			, attenuationRadius(100.0f)
			, direction(vector3(0.0f, -1.0f, 0.0f))
			, falloffExponent(0.001f)
			, castsShadow(false)
			, width(25.0f)
			, height(25.0f)
		{
		}

		virtual void createRenderProxy(SceneProxy* scene) override {
			RectLightProxy* proxy = ALLOC_RENDER_PROXY<RectLightProxy>(scene);

			proxy->positionWS        = getLocation();
			proxy->attenuationRadius = attenuationRadius;
			proxy->directionWS       = glm::normalize(direction);
			proxy->castsShadow       = castsShadow;
			proxy->intensity         = intensity;
			proxy->falloffExponent   = falloffExponent;
			proxy->width             = width;
			proxy->height            = height;

			// Filled later
			proxy->positionVS        = vector3(0.0f); 
			proxy->directionVS       = vector3(0.0f);

			scene->proxyList_rectLight.push_back(proxy);
		}

	public:
		vector3 intensity;
		float   attenuationRadius;
		vector3 direction;
		float   falloffExponent;
		bool    castsShadow;
		float   width;
		float   height;

	};

}
