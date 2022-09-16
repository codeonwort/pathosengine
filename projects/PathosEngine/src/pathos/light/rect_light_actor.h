#pragma once

#include "pathos/actor/actor.h"
#include "pathos/light/rect_light_component.h"

namespace pathos {

	class RectLightActor : public Actor {

	public:
		RectLightActor() {
			lightComponent = createDefaultComponent<RectLightComponent>();
			setAsRootComponent(lightComponent);
		}

		inline RectLightComponent* getLightComponent() const {
			return lightComponent;
		}

		void setDirection(const vector3& direction) {
			lightComponent->direction = normalize(direction);
		}

	private:
		RectLightComponent* lightComponent;

	};

}
