#pragma once

#include "scene_component.h"

// Captures either radiance for indirect specular or irradiance for indirect diffuse.

namespace pathos {

	struct RadianceProbeProxy : public SceneComponentProxy {
		vector3 positionWS;
		float   captureRadius;
	};

	struct IrradianceProbeProxy : public SceneComponentProxy {
		vector3 positionWS;
		float   captureRadius;
	};

	enum class ELightProbeType { Unknown, Radiance, Irradiance };

	class LightProbeComponent : public SceneComponent {

	public:
		virtual void createRenderProxy(SceneProxy* scene) override {
			if (probeType == ELightProbeType::Radiance) {
				RadianceProbeProxy* proxy = ALLOC_RENDER_PROXY<RadianceProbeProxy>(scene);
				proxy->positionWS = getLocation();
				proxy->captureRadius = captureRadius;
				scene->proxyList_radianceProbe.push_back(proxy);
			} else if (probeType == ELightProbeType::Irradiance) {
				IrradianceProbeProxy* proxy = ALLOC_RENDER_PROXY<IrradianceProbeProxy>(scene);
				proxy->positionWS = getLocation();
				proxy->captureRadius = captureRadius;
				scene->proxyList_irradianceProbe.push_back(proxy);
			} else {
				CHECK_NO_ENTRY();
			}
		}

	public:
		ELightProbeType probeType = ELightProbeType::Unknown;
		float captureRadius = 50.0f;

	};

}
