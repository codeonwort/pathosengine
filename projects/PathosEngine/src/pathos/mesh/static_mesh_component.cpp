#include "static_mesh_component.h"
#include "pathos/mesh/mesh.h"
#include "pathos/render/scene_proxy.h"
#include <limits>

namespace pathos {

	AABB calculateWorldBounds(const AABB& localBounds, const matrix4& localToWorld) {
		vector3 minB = localBounds.minBounds;
		vector3 maxB = localBounds.maxBounds;
		vector3 vs[8];
		for (uint32 i = 0; i < 8; ++i) {
			vs[i].x = (i & 1) ? minB.x : maxB.x;
			vs[i].y = ((i >> 1) & 1) ? minB.y : maxB.y;
			vs[i].z = ((i >> 2) & 1) ? minB.z : maxB.z;
		}
		minB = vector3(std::numeric_limits<float>::max());
		maxB = vector3(std::numeric_limits<float>::lowest());
		for (uint32 i = 0; i < 8; ++i) {
			vs[i] = vector3(localToWorld * vector4(vs[i], 1.0f));
			minB = glm::min(minB, vs[i]);
			maxB = glm::max(maxB, vs[i]);
		}
		return AABB::fromMinMax(minB, maxB);
	}

	void StaticMeshComponent::createRenderProxy(SceneProxy* scene) {
		if (mesh == nullptr || getVisibility() == false) {
			return;
		}

		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		const uint32 numSections = static_cast<uint32>(geoms.size());
		constexpr uint8 numMaterialIDs = (uint8)MATERIAL_ID::NUM_MATERIAL_IDS;

		if (castsShadow) {
			for (size_t i = 0u; i < numSections; ++i) {
				MeshGeometry* G = geoms[i];
				Material* M = materials[i];

				ShadowMeshProxy* proxy = ALLOC_RENDER_PROXY<ShadowMeshProxy>(scene);
				proxy->modelMatrix = getLocalMatrix();
				proxy->geometry = geoms[i];
				proxy->worldBounds = calculateWorldBounds(proxy->geometry->getLocalBounds(), proxy->modelMatrix);

				if (M->getMaterialID() == MATERIAL_ID::WIREFRAME) {
					scene->proxyList_wireframeShadowMesh.push_back(proxy);
				} else {
					scene->proxyList_shadowMesh.push_back(proxy);
				}
			}
		}

		for (size_t i = 0u; i < numSections; ++i) {
			MeshGeometry* G = geoms[i];
			Material* M = materials[i];
			uint8 materialID = static_cast<uint8>(M->getMaterialID());
			CHECKF(0 <= materialID && materialID < numMaterialIDs, "Material ID is invalid");

			StaticMeshProxy* proxy = ALLOC_RENDER_PROXY<StaticMeshProxy>(scene);
			proxy->doubleSided = mesh->doubleSided;
			proxy->renderInternal = mesh->renderInternal;
			proxy->modelMatrix = getLocalMatrix();
			proxy->geometry = geoms[i];
			proxy->material = materials[i];
			proxy->worldBounds = calculateWorldBounds(proxy->geometry->getLocalBounds(), proxy->modelMatrix);

			scene->proxyList_staticMesh[static_cast<uint16>(materialID)].push_back(proxy);
		}
	}

	AABB StaticMeshComponent::getWorldBounds() const
	{
		AABB total;
		auto& geoms = mesh->getGeometries();
		if (geoms.size() > 0) {
			total = calculateWorldBounds(geoms[0]->getLocalBounds(), getLocalMatrix());
			for (size_t i = 1; i < geoms.size(); ++i) {
				AABB temp = calculateWorldBounds(geoms[i]->getLocalBounds(), getLocalMatrix());
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
		constexpr uint8 numMaterialIDs = (uint8)MATERIAL_ID::NUM_MATERIAL_IDS;

		for (size_t i = 0u; i < numSections; ++i) {
			MeshGeometry* G = geoms[i];
			Material* M = materials[i];
			uint8 materialID = static_cast<uint8>(M->getMaterialID());
			CHECKF(0 <= materialID && materialID < numMaterialIDs, "Material ID is invalid");

			StaticMeshProxy* proxy = ALLOC_RENDER_PROXY<StaticMeshProxy>(scene);
			proxy->doubleSided = mesh->doubleSided;
			proxy->renderInternal = mesh->renderInternal;
			proxy->modelMatrix = getLocalMatrix();
			proxy->geometry = geoms[i];
			proxy->material = materials[i];
			proxy->worldBounds = calculateWorldBounds(proxy->geometry->getLocalBounds(), proxy->modelMatrix);

			outProxyList.push_back(proxy);
		}
	}

}
