#include "static_mesh_component.h"
#include "pathos/mesh/mesh.h"
#include "pathos/render/scene_proxy.h"

#include "badger/math/hit_test.h"
#include <limits>

namespace pathos {

	void StaticMeshComponent::createRenderProxy(SceneProxy* scene) {
		if (mesh == nullptr || getVisibility() == false) {
			return;
		}

		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		const uint32 numSections = static_cast<uint32>(geoms.size());

		if (castsShadow) {
			for (size_t i = 0u; i < numSections; ++i) {
				ShadowMeshProxy* proxy = ALLOC_RENDER_PROXY<ShadowMeshProxy>(scene);
				proxy->modelMatrix = getLocalMatrix();
				proxy->geometry = geoms[i];
				proxy->material = materials[i];
				proxy->worldBounds = badger::calculateWorldBounds(proxy->geometry->getLocalBounds(), proxy->modelMatrix);
				proxy->doubleSided = mesh->doubleSided;
				proxy->renderInternal = mesh->renderInternal;

				scene->addShadowMeshProxy(proxy);
			}
		}

		for (size_t i = 0u; i < numSections; ++i) {
			StaticMeshProxy* proxy = ALLOC_RENDER_PROXY<StaticMeshProxy>(scene);
			proxy->doubleSided = mesh->doubleSided;
			proxy->renderInternal = mesh->renderInternal;
			proxy->modelMatrix = getLocalMatrix();
			proxy->prevModelMatrix = prevModelMatrix;
			proxy->geometry = geoms[i];
			proxy->material = materials[i];
			proxy->worldBounds = badger::calculateWorldBounds(proxy->geometry->getLocalBounds(), proxy->modelMatrix);

			scene->addStaticMeshProxy(proxy);
		}

		prevModelMatrix = getLocalMatrix();
	}

	AABB StaticMeshComponent::getWorldBounds() const {
		AABB total = AABB::fromMinMax(vector3(FLT_MAX), vector3(-FLT_MAX));
		auto& geoms = mesh->getGeometries();
		if (geoms.size() > 0) {
			total = badger::calculateWorldBounds(geoms[0]->getLocalBounds(), getLocalMatrix());
			for (size_t i = 1; i < geoms.size(); ++i) {
				AABB temp = badger::calculateWorldBounds(geoms[i]->getLocalBounds(), getLocalMatrix());
				total = temp + total;
			}
		}
		return total;
	}

	void StaticMeshComponent::createRenderProxy_internal(SceneProxy* scene, std::vector<StaticMeshProxy*>& outProxyList) {
		if (mesh == nullptr || getVisibility() == false) {
			return;
		}

		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		const uint32 numSections = static_cast<uint32>(geoms.size());

		for (size_t i = 0u; i < numSections; ++i) {
			MeshGeometry* G = geoms[i];
			Material* M = materials[i];

			StaticMeshProxy* proxy = ALLOC_RENDER_PROXY<StaticMeshProxy>(scene);
			proxy->doubleSided = mesh->doubleSided;
			proxy->renderInternal = mesh->renderInternal;
			proxy->modelMatrix = getLocalMatrix();
			proxy->geometry = G;
			proxy->material = M;
			proxy->worldBounds = badger::calculateWorldBounds(G->getLocalBounds(), proxy->modelMatrix);

			outProxyList.push_back(proxy);
		}
	}

}
