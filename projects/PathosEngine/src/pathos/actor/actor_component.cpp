#include "actor_component.h"
#include "actor.h"

namespace pathos {

	void ActorComponent::unregisterFromParent() {
		if (owner != nullptr) {
			owner->unregisterComponent(this);
		}
	}

}
