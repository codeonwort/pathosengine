#include "gltf_loader.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"

#include "tiny_gltf.h"

namespace pathos {

	bool GLTFLoader::loadASCII(const std::string& inFilename) {
		std::string filename = ResourceFinder::get().find(inFilename);

		tinygltf::TinyGLTF tinyLoader;
		tinygltf::Model model;
		std::string err, warn;

		bool ret = tinyLoader.LoadASCIIFromFile(&model, &err, &warn, filename);

		if (!warn.empty()) {
			LOG(LogWarning, warn.c_str());
		}
		if (!err.empty()) {
			LOG(LogError, err.c_str());
		}
		if (!ret) {
			LOG(LogError, "[GLTF] Failed to parse: %s", filename.c_str());
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		
		// #todo-gltf: Convert gltf objects into pathos objects
		//
		// - Relationship of Accessor, Buffer, BufferView?
		// - Node and Scene?
		//
		// parseMaterials();
		//     loadTextures();
		// parseMeshes();
		//     separateBuffers();

		//std::vector<Accessor> accessors;
		//std::vector<Animation> animations;
		//std::vector<Buffer> buffers;
		//std::vector<BufferView> bufferViews;
		//std::vector<Material> materials;
		//std::vector<Mesh> meshes;
		//std::vector<Node> nodes;
		//std::vector<Texture> textures;
		//std::vector<Image> images;
		//std::vector<Skin> skins;
		//std::vector<Sampler> samplers;
		//std::vector<Camera> cameras;
		//std::vector<Scene> scenes;
		//std::vector<Light> lights;

		return true;
	}

}
