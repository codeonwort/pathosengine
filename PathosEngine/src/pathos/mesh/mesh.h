#pragma once

#include <vector>
#include <memory>
#include <pathos/core.h>
#include <pathos/mesh/geometry.h>
#include <pathos/mesh/material.h>
#include <pathos/wrapper/transform.h>

using namespace std;

namespace pathos {

	using Geometries = vector<MeshGeometry*>;
	using Materials = vector<shared_ptr<MeshMaterial>>;

	// mesh = physical presence of geometry with material
	class Mesh {
	protected:
		bool doubleSided = false;
		bool renderInternal = false;
		bool castShadow = false;
		Transform transform;
		Geometries geometries;
		Materials materials;
	public:
		Mesh(MeshGeometry* = nullptr, shared_ptr<MeshMaterial> = nullptr);
		virtual ~Mesh();
		Transform& getTransform();
		const Geometries& getGeometries();
		const Materials& getMaterials();
		bool getDoubleSided();
		void setDoubleSided(bool);
		bool getRenderInternal();
		void setRenderInternal(bool);
	};

}