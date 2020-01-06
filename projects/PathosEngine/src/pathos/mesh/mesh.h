#pragma once

#include "pathos/named_object.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/model_transform.h"
#include "pathos/material/material.h"

#include <vector>

namespace pathos {

	using Geometries = std::vector<MeshGeometry*>;
	using Materials = std::vector<Material*>;

	// mesh = physical presence of geometry with material
	class Mesh : public NamedObject {

	public:
		bool castsShadow    = true;
		bool visible        = true;
		bool doubleSided    = false;
		bool renderInternal = false;

	public:
		Mesh(MeshGeometry* = nullptr, Material* = nullptr);
		virtual ~Mesh();
		void add(MeshGeometry*, Material*);

		ModelTransform& getTransform();
		const Geometries& getGeometries();
		const Materials& getMaterials();

		void setMaterial(int index, Material* M) { materials[index] = M; }

	protected:
		ModelTransform transform;
		Geometries geometries;
		Materials materials;

	};

}
