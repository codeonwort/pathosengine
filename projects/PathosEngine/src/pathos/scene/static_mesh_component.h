#pragma once

#include "pathos/scene/scene_component.h"
#include "badger/types/matrix_types.h"
#include "badger/math/aabb.h"

namespace pathos {

	class StaticMesh;
	class MeshGeometry;
	class Material;

	// #todo-renderer: Further decompose
	struct StaticMeshProxy : public SceneComponentProxy {
		uint32 doubleSided : 1;
		uint32 renderInternal : 1;
		matrix4 modelMatrix;
		matrix4 prevModelMatrix;
		MeshGeometry* geometry;
		Material* material;
		AABB worldBounds;

		// Derived in render thread
		bool bInFrustum = true;
		// Derived in SceneProxy::addStaticMeshProxy()
		bool bTrivialDepthOnly = false;
	};

	struct ShadowMeshProxy : public SceneComponentProxy {
		matrix4 modelMatrix;
		MeshGeometry* geometry;
		Material* material;
		AABB worldBounds;
		uint32 doubleSided : 1;
		uint32 renderInternal : 1;

		bool bTrivialDepthOnly = false;
	};

	class StaticMeshComponent : public SceneComponent {
		friend class Scene; // #todo-godray: due to createRenderProxy_internal()

	public:
		virtual void createRenderProxy(SceneProxy* scene) override;

		inline StaticMesh* getStaticMesh() const { return mesh; }
		inline void setStaticMesh(StaticMesh* inMesh) { mesh = inMesh; }

		AABB getWorldBounds() const;

	private:
		// #todo-godray: Hack
		void createRenderProxy_internal(SceneProxy* scene, std::vector<StaticMeshProxy*>& outProxyList);

	public:
		bool castsShadow = true;

	private:
		// #todo: Need to release StaticMesh so that it releases Geometry and Material instances.
		StaticMesh* mesh = nullptr;
		matrix4 prevModelMatrix;

	};

}
