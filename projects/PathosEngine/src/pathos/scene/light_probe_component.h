#pragma once

#include "scene_component.h"
#include "pathos/smart_pointer.h"
#include "pathos/render/render_target.h"

// Captures either radiance for indirect specular or irradiance for indirect diffuse.

namespace pathos {

	extern const uint32 radianceProbeCubemapSize;
	extern const uint32 radianceProbeNumMips;
	constexpr uint32 radianceProbeMaxCount = 10;

	extern const uint32 irradianceProbeTileSize;
	extern const uint32 irradianceProbeTileCountX;
	extern const uint32 irradianceProbeTileCountY;
	extern const RenderTargetFormat irradianceProbeFormat;

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
		vector4           irradianceTileBounds;
		uint32            irradianceTileID;
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
		uniquePtr<RenderTargetCube> radianceCubemap;
		uniquePtr<RenderTargetCube> specularIBL;
		uint32 irradianceTileID = 0xffffffff;
		vector2ui irradianceRenderOffset = vector2ui(0, 0);
		vector4 irradianceTileBounds = vector4(0.0f);

	};

}
