#pragma once

#ifndef SCENE_H
#define SCENE_H

// Engine
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/envmap.h"

// STL
#include <vector>

namespace pathos {

	// Represents a 3D scene.
	struct Scene {
	public:

		Skybox* skybox = nullptr;
		std::vector<pathos::Mesh*> meshes;

		void add(Mesh*);
		void add(std::initializer_list<Mesh*> meshes);

	};

}

#endif SCENE_H