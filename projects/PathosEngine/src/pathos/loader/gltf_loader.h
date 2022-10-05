#pragma once

#include "badger/types/noncopyable.h"
#include "badger/types/vector_types.h"

#include <string>
#include <vector>

namespace tinygltf { class Model; }

namespace pathos {

	class Material;
	class Mesh;

	struct GLTFModelDesc {
		Mesh* mesh = nullptr;
		vector3 translation = vector3(0.0f);
		vector3 scale = vector3(1.0f);
	};

	class GLTFLoader final : public Noncopyable {
	public:
		GLTFLoader() = default;
		~GLTFLoader() = default;

		bool loadASCII(const char* inFilename);

		size_t numModels() const { return finalModels.size(); }
		const GLTFModelDesc& getModel(size_t ix) const { return finalModels[ix]; }

	private:
		void parseMaterials(tinygltf::Model* tinyModel);
		void parseMeshes(tinygltf::Model* tinyModel);

	private:
		std::vector<Material*> materials;
		std::vector<Mesh*> meshes;

		Material* fallbackMaterial = nullptr;
		std::vector<GLTFModelDesc> finalModels;
	};

}
