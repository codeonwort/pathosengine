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
		friend class Scene; // #todo-godray: due to createRenderProxy_internal()

	public:
		virtual void createRenderProxy(SceneProxy* scene) override;

		inline Mesh* getStaticMesh() const { return mesh; }
		inline void setStaticMesh(Mesh* inMesh) { mesh = inMesh; }

	private:
		// #todo-godray: Hack
		void createRenderProxy_internal(SceneProxy* scene, std::vector<StaticMeshProxy*>& outProxyList);

	public:
		bool castsShadow = true;

	private:
		Mesh* mesh = nullptr;

	};

}
