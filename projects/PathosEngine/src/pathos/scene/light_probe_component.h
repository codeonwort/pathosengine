#pragma once

#include "scene_component.h"
#include "pathos/smart_pointer.h"
#include "pathos/render/render_target.h"

// Captures either radiance for indirect specular or irradiance for indirect diffuse.

namespace pathos {

	extern const uint32 radianceProbeCubemapSize;
	extern const uint32 radianceProbeNumMips;
	constexpr uint32 radianceProbeMaxCount = 10;

	class RenderTargetCube;
	class RenderTarget2D;

	struct RadianceProbeProxy : public SceneComponentProxy {
		vector3           positionWS;
		float             captureRadius;
		RenderTargetCube* renderTarget; // #todo-light-probe: Not thread-safe but do it anyway. Fix later.
		RenderTargetCube* specularIBL;
	};

	// #todo-light-probe: Rename to ReflectionProbeComponent
	class LightProbeComponent : public SceneComponent {

	public:
		LightProbeComponent();
		~LightProbeComponent();

		virtual void createRenderProxy(SceneProxy* scene) override;

		void captureScene(uint32 faceIndex);
		void bakeIBL();

	public:
		float captureRadius = 50.0f;

	private:
		uniquePtr<RenderTargetCube> radianceCubemap;
		uniquePtr<RenderTargetCube> specularIBL;

	};

}
