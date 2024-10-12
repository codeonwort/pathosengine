#pragma once

#include "actor.h"

namespace pathos {

	class LandscapeComponent;
	struct ImageBlob;

	// Specialized actor for landscape mesh.
	class LandscapeActor : public Actor {

	public:
		LandscapeActor();

		void initializeSectors(float inSizeX, float inSizeY, int32 inCountX, int32 inCountY);

		void initializeHeightMap(ImageBlob* heightMapBlob);

		float getLandscapeY(float x, float z) const;

		vector2 getLandscapeUV(float x, float z) const;

		inline LandscapeComponent* getLandscapeComponent() const { return landscapeComponent; }

	public:
		virtual void onSpawn() override;
		virtual void onDestroy() override;
		virtual void onTick(float deltaSeconds) override;

	private:
		LandscapeComponent* landscapeComponent;

	};

}
