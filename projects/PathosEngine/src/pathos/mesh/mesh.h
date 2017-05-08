#pragma once

#include "pathos/core.h"
#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"
#include "pathos/wrapper/transform.h"
#include <vector>
#include <memory>

using namespace std;

namespace pathos {

	using Geometries = vector<MeshGeometry*>;
	using Materials = vector<MeshMaterial*>;

	// mesh = physical presence of geometry with material
	class Mesh : public NamedObject {
	protected:
		bool doubleSided = false;
		bool renderInternal = false;
		bool castShadow = false;
		Transform transform;
		Geometries geometries;
		Materials materials;
	public:
		Mesh(MeshGeometry* = nullptr, MeshMaterial* = nullptr);
		virtual ~Mesh();
		void add(MeshGeometry*, MeshMaterial*);

		Transform& getTransform();
		const Geometries& getGeometries();
		const Materials& getMaterials();
		bool getDoubleSided();
		void setDoubleSided(bool);
		bool getRenderInternal();
		void setRenderInternal(bool);
	};

}