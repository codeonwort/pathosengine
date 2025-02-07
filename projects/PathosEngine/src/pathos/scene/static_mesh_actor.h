#pragma once

#include "static_mesh_component.h"
#include "pathos/scene/actor.h"

namespace pathos {

	class StaticMesh;

	class StaticMeshActor : public Actor {

	public:
		StaticMeshActor() {
			meshComponent = createDefaultComponent<StaticMeshComponent>();

			setAsRootComponent(meshComponent);
		}

		void setStaticMesh(StaticMesh* inMesh) {
			meshComponent->setStaticMesh(inMesh);
		}

		inline StaticMeshComponent* getStaticMeshComponent() const { return meshComponent; }
		inline StaticMesh* getStaticMesh() const { return meshComponent->getStaticMesh(); }

	private:
		StaticMeshComponent* meshComponent;

	};

}
