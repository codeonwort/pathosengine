#include "light_probe_component.h"
#include "pathos/render/render_target.h"

#define RADIANCE_PROBE_CUBEMAP_SIZE    256
#define RADIANCE_PROBE_FORMAT          RenderTargetFormat::RGBA16F

namespace pathos {

	// Reminder for myself: They should be here for uniquePtr + forward decl to work.
	LightProbeComponent::LightProbeComponent() {}
	LightProbeComponent::~LightProbeComponent() {}

	void LightProbeComponent::createRenderProxy(SceneProxy* scene) {
		if (renderTarget == nullptr) {
			renderTarget = makeUnique<RenderTargetCube>();
			renderTarget->respecTexture(RADIANCE_PROBE_CUBEMAP_SIZE, RADIANCE_PROBE_FORMAT, "RadianceProbe");
		}

		if (probeType == ELightProbeType::Radiance) {
			RadianceProbeProxy* proxy = ALLOC_RENDER_PROXY<RadianceProbeProxy>(scene);
			proxy->positionWS = getLocation();
			proxy->captureRadius = captureRadius;
			proxy->renderTarget = renderTarget.get();

			scene->proxyList_radianceProbe.push_back(proxy);
		} else if (probeType == ELightProbeType::Irradiance) {
			IrradianceProbeProxy* proxy = ALLOC_RENDER_PROXY<IrradianceProbeProxy>(scene);
			proxy->positionWS = getLocation();
			proxy->captureRadius = captureRadius;
			proxy->renderTarget = renderTarget.get();

			scene->proxyList_irradianceProbe.push_back(proxy);
		} else {
			CHECK_NO_ENTRY();
		}
	}

}
