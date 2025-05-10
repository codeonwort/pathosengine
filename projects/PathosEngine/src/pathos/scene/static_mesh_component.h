#pragma once

#include "pathos/scene/scene_component.h"
#include "badger/types/matrix_types.h"
#include "pathos/smart_pointer.h"
#include "badger/math/aabb.h"

namespace pathos {

	class StaticMesh;
	class MeshGeometry;
	class Material;
	class MaterialProxy;

	// #todo-renderer: Further decompose
	struct StaticMeshProxy : public SceneComponentProxy {
		uint32         doubleSided : 1;
		uint32         renderInternal : 1;
		matrix4        modelMatrix;
		matrix4        prevModelMatrix;
		MeshGeometry*  geometry;
		MaterialProxy* material;
		AABB           worldBounds;

		bool           bInFrustum = true; // Derived in render thread
		bool           bTrivialDepthOnly = false; // Derived in SceneProxy::addStaticMeshProxy()
	};

	struct ShadowMeshProxy : public SceneComponentProxy {
		matrix4        modelMatrix;
		MeshGeometry*  geometry;
		MaterialProxy* material;
		AABB           worldBounds;
		uint32         doubleSided : 1;
		uint32         renderInternal : 1;

		bool           bTrivialDepthOnly = false;
	};

	class StaticMeshComponent : public SceneComponent {
		friend class Scene; // #todo-godray: due to createRenderProxy_internal()

	public:
		virtual void createRenderProxy(SceneProxy* scene) override;

		inline assetPtr<StaticMesh> getStaticMesh() const { return mesh; }
		inline void setStaticMesh(assetPtr<StaticMesh> inMesh) { mesh = inMesh; }

		AABB getWorldBounds() const;

	private:
		// #todo-godray: Hack
		void createRenderProxy_internal(SceneProxy* scene, std::vector<StaticMeshProxy*>& outProxyList);

	public:
		bool castsShadow = true;

	private:
		assetPtr<StaticMesh> mesh;
		matrix4 prevModelMatrix;

	};

}
