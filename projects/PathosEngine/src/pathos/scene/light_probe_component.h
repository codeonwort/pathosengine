#pragma once

#include "scene_component.h"
#include "pathos/smart_pointer.h"

// Captures either radiance for indirect specular or irradiance for indirect diffuse.

namespace pathos {

	extern const uint32 radianceProbeCubemapSize;
	extern const uint32 radianceProbeNumMips;
	constexpr uint32 radianceProbeMaxCount = 10;
	extern const uint32 irradianceProbeCubemapSize;

	class RenderTargetCube;
	class RenderTarget2D;

	struct RadianceProbeProxy : public SceneComponentProxy {
		vector3           positionWS;
		float             captureRadius;
		RenderTargetCube* renderTarget; // #todo-light-probe: Not thread-safe but do it anyway. Fix later.
		RenderTargetCube* specularIBL;
	};

	struct IrradianceProbeProxy : public SceneComponentProxy {
		vector3           positionWS;
		float             captureRadius;
		RenderTargetCube* renderTarget;
	};

	enum class ELightProbeType { Unknown, Radiance, Irradiance };

	class LightProbeComponent : public SceneComponent {

	public:
		LightProbeComponent();
		~LightProbeComponent();

		virtual void createRenderProxy(SceneProxy* scene) override;

		void captureScene(uint32 faceIndex);
		void bakeIBL();

	public:
		ELightProbeType probeType = ELightProbeType::Unknown;
		float captureRadius = 50.0f;

	private:
		uniquePtr<RenderTargetCube> renderTarget; // Radiance capture
		uniquePtr<RenderTargetCube> bakedIBL;     // Specular or diffuse IBL
		
		uniquePtr<RenderTarget2D> irradianceAtlas;

	};

}
