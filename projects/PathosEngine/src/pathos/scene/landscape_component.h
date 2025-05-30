#pragma once

#include "pathos/scene/scene_component.h"
#include "pathos/smart_pointer.h"
#include "badger/math/aabb.h"

#include <vector>

namespace pathos {

	class MeshGeometry;
	class Material;
	class MaterialProxy;
	class Buffer;
	class Texture;
	struct ImageBlob;

	struct LandscapeProxy : public SceneComponentProxy {
		Buffer*        indirectDrawArgsBuffer;
		Buffer*        indirectDrawCountBuffer;
		Buffer*        sectorParameterBuffer;
		MeshGeometry*  geometry;
		MaterialProxy* material;
		matrix4        modelMatrix;
		matrix4        prevModelMatrix;
		uint32         indirectDrawCount;
		bool           bGpuDriven;
		// For gpu driven
		vector4ui      indexCountPerLOD;
		vector4ui      firstIndexPerLOD;
		vector3        actorPosition;
		float          sectorSizeX;
		vector3        cameraPosition;
		float          sectorSizeY;
		uint32         sectorCountX;
		uint32         sectorCountY;
		float          cullDistance;
		float          heightMultiplier;
	};

	class LandscapeComponent : public SceneComponent {

	public:
		LandscapeComponent();
		~LandscapeComponent();

		void initializeSectors(float inSizeX, float inSizeY, int32 inCountX, int32 inCountY);

		// Prepare heightmap data for sampleHeightmap().
		void initializeHeightMap(ImageBlob* blob);

		// Landscape uses its internal material, so only configure texture parameters.
		inline void setAlbedoTexture(Texture* texture) { albedoTexture = texture; }
		inline void setHeightmapTexture(Texture* texture) { heightmapTexture = texture; }
		inline void setNormalmapTexture(Texture* texture) { normalmapTexture = texture; }

		inline void setGpuDriven(bool enable) { bGpuDriven = enable; }

		vector2 getNormalizedUV(float x, float z) const;

		float sampleHeightmap(float u, float v) const; // [0, 1]

		// Default value is 1.0.
		inline float getHeightMultiplier() const { return heightMultiplier; }
		inline void setHeightMultiplier(float multiplier) { heightMultiplier = multiplier; }

		inline float getCullDistance() const { return cullDistance; }
		inline void setCullDistance(float value) { cullDistance = value; }

		//~ BEGIN ActorComponent interface
		virtual void createRenderProxy(SceneProxy* scene) override;
		//~ END ActorComponent interface

	private:
		void updateMaterial();
		uint32 fillIndirectDrawBuffers(SceneProxy* scene); // CPU version

		bool bGpuDriven = true;

		uniquePtr<MeshGeometry> geometry; // All LODs in a single mesh
		std::vector<uint32> numVertices;  // Per LOD
		std::vector<uint32> numIndices;   // Per LOD
		std::vector<int32> indexOffsets;  // Per LOD

		// To sample heightmap in CPU
		uint32 heightMapWidth = 0;
		uint32 heightMapHeight = 0;
		std::vector<float> heightMapValues;
		float heightMultiplier = 1.0f;

		assetPtr<Material> material;
		Texture* albedoTexture = nullptr;
		Texture* heightmapTexture = nullptr;
		Texture* normalmapTexture = nullptr;
		uniquePtr<Buffer> indirectDrawArgsBuffer; // indirect draw commands
		uniquePtr<Buffer> indirectDrawCountBuffer; // indirect draw count (single element)
		uniquePtr<Buffer> sectorParameterBuffer;
		matrix4 prevModelMatrix = matrix4(1.0f);

		float cullDistance;

		// For sectors
		float sizeX = 0.0f;
		float sizeY = 0.0f;
		int32 countX = 0;
		int32 countY = 0;

	};

}
