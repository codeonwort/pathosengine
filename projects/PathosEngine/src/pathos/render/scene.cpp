#include "scene.h"

namespace pathos {

	void Scene::add(Mesh* mesh) {
		meshes.push_back(mesh);
	}

	void Scene::add(std::initializer_list<Mesh*> meshes) {
		for (Mesh* mesh : meshes) {
			this->meshes.push_back(mesh);
		}
	}
}