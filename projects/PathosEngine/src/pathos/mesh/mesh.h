#pragma once

#include "pathos/named_object.h"
#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"
#include "pathos/wrapper/transform.h"

#include <vector>

namespace pathos {

	using Geometries = std::vector<MeshGeometry*>;
	using Materials = std::vector<MeshMaterial*>;

	// mesh = physical presence of geometry with material
	class Mesh : public NamedObject {

	public:
		bool castsShadow    = true;
		bool visible        = true;
		bool doubleSided    = false;
		bool renderInternal = false;

	public:
		Mesh(MeshGeometry* = nullptr, MeshMaterial* = nullptr);
		virtual ~Mesh();
		void add(MeshGeometry*, MeshMaterial*);

		Transform& getTransform();
		const Geometries& getGeometries();
		const Materials& getMaterials();

		void setMaterial(int index, MeshMaterial* M) { materials[index] = M; }

	protected:
		Transform transform;
		Geometries geometries;
		Materials materials;

	};

}
