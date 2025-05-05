#pragma once

#include "scene_component.h"
#include "pathos/render/render_target.h"
#include "pathos/smart_pointer.h"

// Captures either radiance for indirect specular or irradiance for indirect diffuse.

namespace pathos {

	// #todo-light-probe: Not here
	extern const uint32 reflectionProbeCubemapSize;
	extern const uint32 reflectionProbeNumMips;
	constexpr uint32 reflectionProbeMaxCount = 340;

	class RenderTargetCube;
	class RenderTarget2D;
	class Texture;

	struct ReflectionProbeProxy : public SceneComponentProxy {
		vector3           positionWS;
		float             captureRadius;
		uint32            cubemapIndex;
		RenderTargetCube* renderTarget; // #todo-light-probe: Not thread-safe but do it anyway. Fix later.
		RenderTargetCube* specularIBL;
	};

	class ReflectionProbeComponent : public SceneComponent {

	public:
		ReflectionProbeComponent();
		~ReflectionProbeComponent();

		virtual void createRenderProxy(SceneProxy* scene) override;

		inline uint32 getCubemapIndex() const { return cubemapIndex; }
		inline void setCubemapIndex(uint32 index) { cubemapIndex = index; }

		void captureScene(uint32 faceIndex);
		void bakeIBL(Texture* cubemapArray);

	public:
		float captureRadius = 50.0f;

	private:
		uniquePtr<RenderTargetCube> radianceCubemap;
		uniquePtr<RenderTargetCube> specularIBL;
		uint32 cubemapIndex = 0xffffffff;

	};

}
