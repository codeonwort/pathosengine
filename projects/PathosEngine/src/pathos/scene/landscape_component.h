#pragma once

#include "pathos/scene/scene_component.h"
#include "pathos/smart_pointer.h"
#include "badger/math/aabb.h"

#include <vector>

namespace pathos {

	class MeshGeometry;
	class Material;
	class Buffer;

	struct LandscapeProxy : public SceneComponentProxy {
		Buffer* indirectDrawArgsBuffer;
		Buffer* sectorParameterBuffer;
		MeshGeometry* geometry;
		Material* material;
		uint32 indirectDrawCount;
		matrix4 modelMatrix;
		matrix4 prevModelMatrix;
	};

	class LandscapeComponent : public SceneComponent {

	public:
		LandscapeComponent();
		~LandscapeComponent();

		void initializeSectors(float inSizeX, float inSizeY, int32 inCountX, int32 inCountY);

		inline void setMaterial(Material* inMaterial) { material = inMaterial; }

		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		uniquePtr<MeshGeometry> geometry; // All LODs in a single mesh
		std::vector<uint32> numVertices;  // Per LOD
		std::vector<uint32> numIndices;   // Per LOD
		std::vector<int32> indexOffsets;  // Per LOD

		Material* material = nullptr;
		uniquePtr<Buffer> indirectDrawArgsBuffer;
		uniquePtr<Buffer> sectorParameterBuffer;
		matrix4 prevModelMatrix = matrix4(1.0f);

		// For sectors
		float sizeX = 0.0f;
		float sizeY = 0.0f;
		int32 countX = 0;
		int32 countY = 0;

	};

}
