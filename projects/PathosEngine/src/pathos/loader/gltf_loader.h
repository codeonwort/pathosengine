#pragma once

#include "pathos/rhi/gl_handles.h"

#include "badger/types/noncopyable.h"
#include "badger/types/vector_types.h"
#include "badger/math/rotator.h"

#include <string>
#include <vector>

namespace tinygltf { class TinyGLTF; class Model; }

namespace pathos {

	class Texture;
	class Material;
	class Mesh;
	class MeshGeometry;
	class Actor;
	struct ImageBlob;

	struct GLTFModelDesc {
		Mesh* mesh = nullptr;
		vector3 translation = vector3(0.0f);
		vector3 scale = vector3(1.0f);
		Rotator rotation;
		bool bReferencedByScene = false;
	};

	struct GLTFPendingTexture {
		ImageBlob* blob = nullptr;
		bool sRGB = false;
		std::string debugName;
		Texture* glTexture = nullptr;
	};

	struct GLTFPendingTextureParameter {
		GLTFPendingTextureParameter(
			Material* inMaterial, const std::string& inParam, uint32 inIndex, Texture* inFallback)
			: material(inMaterial)
			, parameterName(inParam)
			, index(inIndex)
			, fallbackTexture(inFallback)
		{}
		Material* material;
		std::string parameterName;
		uint32 index;
		Texture* fallbackTexture;
	};
	struct GLTFPendingGeometry {
		MeshGeometry* geometry;

		void* indexBlob;
		uint32 indexLength;
		bool bShouldFreeIndex;
		bool bIndex16;

		void* positionBlob;
		uint32 positionLength;
		bool bShouldFreePosition;

		void* uvBlob;
		uint32 uvLength;
		bool bShouldFreeUV;
		bool bFlipTexcoordY;

		void* normalBlob;
		uint32 normalLength;
		bool bShouldFreeNormal;
	};

	class GLTFLoader final : public Noncopyable {
	public:
		GLTFLoader();
		~GLTFLoader();

		inline bool isValid() const { return bIsValid; } // Use this to check if load was successful.

		bool loadASCII(const char* inFilename);

		void finalizeGPUUpload();

		// Craft StaticMeshComponents and attach to the actor.
		void attachToActor(Actor* targetActor);

		// NOTE: Should finalize first.
		size_t numModels() const { return finalModels.size(); }
		const GLTFModelDesc& getModel(size_t ix) const { return finalModels[ix]; }

	private:
		void parseTextures(tinygltf::Model* tinyModel);
		void parseMaterials(tinygltf::Model* tinyModel);
		void parseMeshes(tinygltf::Model* tinyModel);
		void checkSceneReference(tinygltf::Model* tinyModel, int32 sceneIndex, std::vector<GLTFModelDesc>& finalModels);

	private:
		uniquePtr<tinygltf::TinyGLTF> tinyLoader;
		uniquePtr<tinygltf::Model> tinyModel;
		bool bIsValid = false;

		std::vector<GLTFPendingTexture> pendingTextures;
		std::vector<GLTFPendingTextureParameter> pendingTextureParameters;
		std::vector<GLTFPendingGeometry> pendingGeometries;

		std::vector<Material*> materials;
		std::vector<Mesh*> meshes;

		Material* fallbackMaterial = nullptr;
		std::vector<GLTFModelDesc> finalModels;
		std::vector<int32> transformParentIx;
	};

}
