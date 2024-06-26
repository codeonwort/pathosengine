#pragma once

#include "pathos/scene/scene_component.h"
#include <algorithm>

namespace pathos {

	struct RectLightProxy : public SceneComponentProxy {
		// Center of rectangular source region.
		// NOTE: This is not the position as the light source's origin.
		vector3   positionVS;
		float     attenuationRadius;

		vector3   directionVS;
		uint32    castsShadow;

		vector3   intensity; // Multiplied by color
		float     falloffExponent;

		vector3   upVS;
		float     halfHeight;
		vector3   rightVS;
		float     halfWidth;
	};

	class RectLightComponent : public SceneComponent {

	public:
		RectLightComponent()
			: color(vector3(1.0f, 1.0f, 1.0f))
			, intensity(100.0f)
			, attenuationRadius(1.0f)
			, falloffExponent(0.001f)
			, castsShadow(false)
			, width(0.25f)
			, height(0.25f)
		{
		}

		virtual void createRenderProxy(SceneProxy* scene) override {
			RectLightProxy* proxy = ALLOC_RENDER_PROXY<RectLightProxy>(scene);

			matrix3 rotation = matrix3(getRotation().toMatrix());
			vector3 forward = getRotation().toDirection();

			// NOTE: Vectors are in WS here, but converted to VS later.
			proxy->positionVS        = getLocation();
			proxy->attenuationRadius = attenuationRadius;
			proxy->directionVS       = forward;
			proxy->castsShadow       = castsShadow;
			proxy->intensity         = color * intensity;
			proxy->falloffExponent   = falloffExponent;
			proxy->halfWidth         = 0.5f * width;
			proxy->halfHeight        = 0.5f * height;
			proxy->upVS              = rotation * vector3(0.0f, 1.0f, 0.0f);
			proxy->rightVS           = rotation * vector3(0.0f, 0.0f, 1.0f);

			scene->proxyList_rectLight.push_back(proxy);
		}

	public:
		vector3 color;             // Luminous efficiency function. Should be clamped to [0, 1]
		float   intensity;         // Unit: candela = lm/sr = luminuous intensity
		float   attenuationRadius; // Unit: meter
		float   falloffExponent;
		bool    castsShadow;
		float   width;             // Unit: meter
		float   height;            // Unit: meter

	};

}
