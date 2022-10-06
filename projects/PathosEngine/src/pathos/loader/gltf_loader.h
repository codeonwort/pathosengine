#pragma once

#include "pathos/gl_handles.h"

#include "badger/types/noncopyable.h"
#include "badger/types/vector_types.h"
#include "badger/math/rotator.h"

#include <string>
#include <vector>

namespace tinygltf { class Model; }

namespace pathos {

	class Material;
	class Mesh;
	class Actor;

	struct GLTFModelDesc {
		Mesh* mesh = nullptr;
		vector3 translation = vector3(0.0f);
		vector3 scale = vector3(1.0f);
		Rotator rotation;
	};

	class GLTFLoader final : public Noncopyable {
	public:
		GLTFLoader() = default;
		~GLTFLoader() = default;

		// #todo-gltf: GLTF spec limits emissive factor to 1.0??? Then how to do HDR lighting?
		float emissiveBoost = 1.0f;

		bool loadASCII(const char* inFilename);

		// Craft StaticMeshComponents and attach to the actor.
		void attachToActor(Actor* targetActor);

		size_t numModels() const { return finalModels.size(); }
		const GLTFModelDesc& getModel(size_t ix) const { return finalModels[ix]; }

	private:
		void parseTextures(tinygltf::Model* tinyModel);
		void parseMaterials(tinygltf::Model* tinyModel);
		void parseMeshes(tinygltf::Model* tinyModel);

	private:
		std::vector<GLuint> glTextures;
		std::vector<Material*> materials;
		std::vector<Mesh*> meshes;

		Material* fallbackMaterial = nullptr;
		std::vector<GLTFModelDesc> finalModels;
		std::vector<int32> transformParentIx;
	};

}
