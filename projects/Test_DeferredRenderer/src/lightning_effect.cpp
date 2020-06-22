#include "lightning_effect.h"

#include "pathos\mesh\mesh.h"
#include "pathos\mesh\static_mesh_component.h"
#include "pathos\mesh\geometry_primitive.h"
#include "pathos\mesh\geometry_procedural.h"

LightningActor::LightningActor()
{
	sphereComponent = createDefaultComponent<StaticMeshComponent>();

	sphereGeometry = new SphereGeometry(1.0f, 50);
	sphereMaterial = new ColorMaterial;
	sphereMaterial->setAlbedo(0.2f, 0.3f, 0.8f);
	sphereMaterial->setRoughness(0.0f);
	sphereMaterial->setMetallic(0.0f);
	sphereMaterial->setEmissive(3.0f, 3.0f, 5.0f);
	sphereComponent->setStaticMesh(new Mesh(sphereGeometry, sphereMaterial));

	setAsRootComponent(sphereComponent);
	
	particleComponent = createDefaultComponent<LightningParticleComponent>();
}

void LightningActor::generateParticle(const vector3& p0, const vector3& p1)
{
	particleComponent->generateParticle(p0, p1);
}

LightningParticleComponent::LightningParticleComponent()
{
	G = new ProceduralGeometry;

	M = new ColorMaterial;
	M->setAlbedo(0.0f, 0.0f, 0.0f);
	M->setRoughness(0.0f);
	M->setMetallic(0.0f);
	M->setEmissive(2.0f, 2.0f, 2.0f);
	M->billboard = true;

	setStaticMesh(new Mesh(G, M));
	getStaticMesh()->doubleSided = true;
}

void LightningParticleComponent::generateParticle(const vector3& p0, const vector3& p1)
{
	G->clear();

	std::vector<vector3> positions;
	std::vector<vector2> uvs;
	std::vector<uint32> indices;

	positions.push_back(p0);
	positions.push_back(p0);
	positions.push_back(p1);
	positions.push_back(p1);

	uvs.push_back(vector2(0.0f, 0.0f));
	uvs.push_back(vector2(1.0f, 0.0f));
	uvs.push_back(vector2(0.0f, 0.0f));
	uvs.push_back(vector2(1.0f, 0.0f));

	indices.push_back(0); indices.push_back(1); indices.push_back(2);
	indices.push_back(1); indices.push_back(3); indices.push_back(2);
	
	G->updatePositionData((float*)positions.data(), (uint32)(positions.size() * 3));
	G->updateUVData((float*)uvs.data(), (uint32)(uvs.size() * 2));
	G->updateIndexData(indices.data(), (uint32)indices.size());
	G->calculateNormals();
	G->calculateTangentBasis();
}
