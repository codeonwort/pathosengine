#pragma once

#include "pathos/core.h"
#include "pathos/mesh/geometry.h"
#include "pathos/material/material.h"
#include "pathos/wrapper/transform.h"
#include <vector>
#include <memory>

using namespace std;

namespace pathos {

	using Geometries = std::vector<MeshGeometry*>;
	using Materials = std::vector<MeshMaterial*>;

	// mesh = physical presence of geometry with material
	class Mesh : public NamedObject {

	protected:
		bool doubleSided = false;
		bool renderInternal = false;
		bool castShadow = false;
		bool visible = true;
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

		void setMaterial(int index, MeshMaterial* M) { materials[index] = M; }

		inline bool getVisible() const { return visible; }
		bool getDoubleSided();
		bool getRenderInternal();

		inline void setVisible(bool value) { visible = value; }
		void setDoubleSided(bool);
		void setRenderInternal(bool);

	};

}