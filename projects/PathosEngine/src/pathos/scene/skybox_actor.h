#pragma once

#include "pathos/scene/sky_actor.h"
#include "pathos/scene/skybox_component.h"

namespace pathos {

	class Texture;
	class Material;

	// Render sky with cubemap texture or sky material.
	// If both are set, the one that is most recently set is used.
	class SkyboxActor : public SkyActor {

	public:
		SkyboxActor() {
			component = createDefaultComponent<SkyboxComponent>();
			setAsRootComponent(component);
		}

		void setCubemapTexture(Texture* texture, float lod = 0.0f);

		void setIntensityMultiplier(float multiplier);

		void setSkyboxMaterial(Material* material);

		inline SkyboxComponent* getSkyComponent() const { return component; }

	private:
		SkyboxComponent* component;
	};

}
