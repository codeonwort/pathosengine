#include "static_mesh_component.h"
#include "pathos/mesh/mesh.h"
#include "pathos/render/scene_proxy.h"

namespace pathos {

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
				proxy->modelMatrix = getMatrix();
				proxy->geometry = geoms[i];

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
			proxy->modelMatrix = getMatrix();
			proxy->geometry = geoms[i];
			proxy->material = materials[i];

			scene->proxyList_staticMesh[static_cast<uint16>(materialID)].push_back(proxy);
		}
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
			proxy->modelMatrix = getMatrix();
			proxy->geometry = geoms[i];
			proxy->material = materials[i];

			outProxyList.push_back(proxy);
		}
	}

}
