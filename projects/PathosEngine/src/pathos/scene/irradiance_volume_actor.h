#pragma once

#include "pathos/scene/actor.h"
#include "pathos/scene/scene_component.h"
#include "pathos/render/render_target.h"
#include "pathos/smart_pointer.h"

namespace pathos {
	extern const uint32 irradianceProbeTileSize;
	extern const uint32 irradianceProbeTileCountX;
	extern const uint32 irradianceProbeTileCountY;
	extern const RenderTargetFormat irradianceProbeFormat;
	constexpr uint32 IRRADIANCE_TILE_INVALID_ID = 0xffffffff;

	struct IrradianceVolumeProxy : public SceneComponentProxy {
		vector3 minBounds;
		uint32 irradianceTileFirstID;
		vector3 maxBounds;
		uint32 numProbes;
		vector3ui gridSize;
	};
}

namespace pathos {

	class IrradianceVolumeActor : public Actor {

	public:
		IrradianceVolumeActor() {
			//
		}

		// Defines the area and probe density of irradiance volume.
		// @param minBounds Volume's min bounds (also this actor's world location).
		// @param maxBounds Volume's max bounds.
		// @param probeGrid The number of probes to place in X/Y/Z axes.
		void initializeVolume(
			const vector3& minBounds,
			const vector3& maxBounds,
			const vector3ui& probeGrid);

		// Update probes in this volume. Each probe needs 7 steps to be fully processed.
		// If there are N probes, this volume needs ((N * 7) / numSteps) frames to be fully updated.
		// @param numSteps The number of update steps to perform.
		void updateProbes(int32 numSteps);

		void internal_createRenderProxy(SceneProxy* sceneProxy) const;

		inline uint32 numProbes() const { return gridSize.x * gridSize.y * gridSize.z; }

		inline bool hasLightingData() const {
			return bVolumeInitialized
				&& (irradianceTileFirstID != IRRADIANCE_TILE_INVALID_ID);
		}

	private:
		vector3 getProbeLocationByIndex(uint32 probeIndex) const;
		vector3 getProbeLocationByCoord(uint32 gridX, uint32 gridY, uint32 gridZ) const;

		void captureFace(uint32 probeIndex, uint32 faceIndex);
		void bakeIrradiance(uint32 probeIndex);

		vector3 minBounds = vector3(0.0f);
		vector3 maxBounds = vector3(0.0f);
		vector3ui gridSize = vector3ui(0, 0, 0);
		bool bVolumeInitialized = false;

		uniquePtr<RenderTargetCube> radianceCubemap;
		float captureRadius = 0.0f;
		uint32 currentUpdateIndex = 0; // Index of probe to update [0, totalProbeCount-1]
		uint32 currentUpdatePhase = 0; // 0~5: capture cube face, 6: integrate

		uint32 irradianceTileFirstID = IRRADIANCE_TILE_INVALID_ID;
	};

}
