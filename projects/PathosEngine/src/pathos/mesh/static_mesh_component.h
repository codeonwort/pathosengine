#pragma once

#include "pathos/actor/scene_component.h"
#include "badger/types/matrix_types.h"

namespace pathos {

	class Mesh;
	class MeshGeometry;
	class Material;

	// #todo-renderer: Further decompose
	struct StaticMeshProxy : public SceneComponentProxy {
		uint32 doubleSided : 1;
		uint32 renderInternal : 1;
		matrix4 modelMatrix;
		MeshGeometry* geometry;
		Material* material;
	};

	struct ShadowMeshProxy : public SceneComponentProxy {
		matrix4 modelMatrix;
		MeshGeometry* geometry;
	};

	class StaticMeshComponent : public SceneComponent {

	public:
		inline Mesh* getStaticMesh() const { return mesh; }

		void setStaticMesh(Mesh* inMesh) { mesh = inMesh; }

		virtual void createRenderProxy(Scene* scene);
		void createRenderProxy_internal(Scene* scene, std::vector<StaticMeshProxy*>& outProxyList);

	private:
		Mesh* mesh;

	};

}
