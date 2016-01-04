//#include <BossEngine/Component/Transform.h>
#include <pathos/mesh/mesh.h>

namespace pathos {

	// Mesh
	Mesh::Mesh(MeshGeometry* geom, shared_ptr<MeshMaterial> mat) {
		doubleSided = false;
		castShadow = true;
		if(geom != nullptr) geometries.push_back(geom);
		if(mat != nullptr) materials.push_back(mat);
	}
	Mesh::~Mesh() {
		for (auto geom : geometries) delete geom;
	}
	Transform& Mesh::getTransform() { return transform; }
	const Geometries& Mesh::getGeometries() { return geometries; }
	const Materials& Mesh::getMaterials() { return materials; }
	bool Mesh::getDoubleSided() { return doubleSided; }
	void Mesh::setDoubleSided(bool b) { doubleSided = b; }
	bool Mesh::getRenderInternal() { return renderInternal; }
	void Mesh::setRenderInternal(bool b) { renderInternal = b; }

}