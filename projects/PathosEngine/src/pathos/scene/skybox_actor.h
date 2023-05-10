#pragma once

#include "pathos/scene/sky_actor.h"
#include "pathos/scene/skybox_component.h"

namespace pathos {

	class Material;

	// Render sky with cubemap texture or sky material.
	// If both are set, the one that is most recently set is used.
	class SkyboxActor : public SkyActor {

	public:
		SkyboxActor() {
			component = createDefaultComponent<SkyboxComponent>();
			setAsRootComponent(component);
		}

		void setCubemapTexture(GLuint textureID, float lod = 0.0f);

		void setSkyboxMaterial(Material* material);

		inline SkyboxComponent* getSkyboxComponent() const { return component; }

	private:
		SkyboxComponent* component;
	};

}
