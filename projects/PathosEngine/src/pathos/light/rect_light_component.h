#pragma once

#include "pathos/actor/scene_component.h"
#include <algorithm>

namespace pathos {

	struct RectLightProxy : public SceneComponentProxy {
		// Center of rectangular source region.
		// NOTE: This is not the position as the light source's origin.
		vector3   positionWS;
		float     attenuationRadius;

		vector3   directionWS;
		uint32    castsShadow;

		vector3   intensity;
		float     falloffExponent;

		float     halfWidth;
		float     halfHeight;
		vector2   _padding0;

		vector3   positionVS;
		float     _padding1;

		vector3   directionVS;
		float     _padding2;

		vector3   upWS;
		float     _padding3;
		vector3   rightWS;
		float     _padding4;
		vector3   upVS;
		float     _padding5;
		vector3   rightVS;
		float     _padding6;
	};

	class RectLightComponent : public SceneComponent {

	public:
		RectLightComponent()
			: intensity(vector3(1.0f))
			, attenuationRadius(100.0f)
			, falloffExponent(0.001f)
			, castsShadow(false)
			, width(25.0f)
			, height(25.0f)
		{
		}

		virtual void createRenderProxy(SceneProxy* scene) override {
			RectLightProxy* proxy = ALLOC_RENDER_PROXY<RectLightProxy>(scene);

			matrix3 rotation = matrix3(getRotation().toMatrix());
			vector3 forward = getRotation().toDirection();

			proxy->positionWS        = getLocation();
			proxy->attenuationRadius = attenuationRadius;
			proxy->directionWS       = forward;
			proxy->castsShadow       = castsShadow;
			proxy->intensity         = intensity;
			proxy->falloffExponent   = falloffExponent;
			proxy->halfWidth         = 0.5f * width;
			proxy->halfHeight        = 0.5f * height;
			proxy->upWS              = rotation * vector3(0.0f, 1.0f, 0.0f);
			proxy->rightWS           = rotation * vector3(0.0f, 0.0f, 1.0f);

			// Filled later
			proxy->positionVS        = vector3(0.0f);
			proxy->directionVS       = vector3(0.0f);
			proxy->upVS              = vector3(0.0f);
			proxy->rightVS           = vector3(0.0f);

			scene->proxyList_rectLight.push_back(proxy);
		}

	public:
		vector3 intensity;

		float   attenuationRadius;
		float   falloffExponent;

		bool    castsShadow;

		float   width;
		float   height;

	};

}
