#include <pathos/mesh/mesh.h>

namespace pathos {

	/**
	* constructor.<br/>
	* each geometry is regarded as uniquely belongs to only one mesh.<br/>
	* each material can be shared among multiple meshes.
	*/
	Mesh::Mesh(MeshGeometry* geom, shared_ptr<MeshMaterial> mat) {
		doubleSided = false;
		castShadow = true;
		if (geom != nullptr || mat != nullptr){
			geometries.push_back(geom);
			materials.push_back(mat);
		}
	}

	/**
	* deconstructor.<br/>
	* it deallocates all geometries it owns. don't share one geometry among two or more meshes!
	*/
	Mesh::~Mesh() {
		for (auto geom : geometries) if(geom != nullptr) delete geom;
	}

	void Mesh::add(MeshGeometry* G, shared_ptr<MeshMaterial> M) {
		geometries.push_back(G);
		materials.push_back(M);
	}

	Transform& Mesh::getTransform() { return transform; }
	const Geometries& Mesh::getGeometries() { return geometries; }
	const Materials& Mesh::getMaterials() { return materials; }
	bool Mesh::getDoubleSided() { return doubleSided; }
	void Mesh::setDoubleSided(bool b) { doubleSided = b; }
	bool Mesh::getRenderInternal() { return renderInternal; }
	void Mesh::setRenderInternal(bool b) { renderInternal = b; }

}