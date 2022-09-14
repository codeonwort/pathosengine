#include "transform_test_actor.h"

#include "pathos/mesh/static_mesh_component.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/mesh.h"

static const float height = 40.0f;
static const float radius = 50.0f;
static const int32 numStars = 7;

TransformTestActor::TransformTestActor()
{
	root = createDefaultComponent<StaticMeshComponent>();
	setAsRootComponent(root);

	MeshGeometry* rootG = new SphereGeometry(10.0f);
	ColorMaterial* rootM = new ColorMaterial;
	rootM->setRoughness(0.35f);
	root->setStaticMesh(new Mesh(rootG, rootM));

	MeshGeometry* starG = new CubeGeometry(vector3(10.0f, 10.0f, 10.0f));
	ColorMaterial* starM = new ColorMaterial;
	starM->setEmissive(1.0f, 1.0f, 5.0f);

	MeshGeometry* moonG = new CubeGeometry(vector3(3.0f, 3.0f, 3.0f));
	ColorMaterial* moonM = new ColorMaterial;
	moonM->setAlbedo(0.9f, 0.9f, 0.1f);

	for (int32 i = 0; i < numStars; ++i) {
		StaticMeshComponent* star = createDefaultComponent<StaticMeshComponent>();
		star->setTransformParent(root);
		stars.push_back(star);

		float angle = (2.0f * 3.141592f) * (float)i / numStars;
		star->setLocation(vector3(radius * std::cos(angle), height, radius * std::sin(angle)));

		star->setStaticMesh(new Mesh(starG, starM));
	}

	for (int32 i = 0; i < numStars; ++i) {
		StaticMeshComponent* moon = createDefaultComponent<StaticMeshComponent>();
		moon->setTransformParent(stars[i]);
		moons.push_back(moon);
	
		moon->setStaticMesh(new Mesh(moonG, moonM));
	}
}

void TransformTestActor::onTick(float deltaSeconds) {
	const float worldTime = gEngine->getWorldTime();

	root->setRotation(Rotator(0.0f, 30.0f * std::cos(worldTime), 0.0f));

	for (int32 i = 0; i < numStars; ++i) {
		StaticMeshComponent* star = stars[i];

		float angle = (2.0f * 3.141592f) * (float)i / numStars;
		angle += worldTime;
		star->setLocation(vector3(radius * std::cos(angle), height, radius * std::sin(angle)));
	}

	for (int32 i = 0; i < numStars; ++i) {
		StaticMeshComponent* moon = moons[i];
		
		moon->setRotation(Rotator(worldTime * 0.89f, worldTime * 1.27f, 0.0f));

		float angle = (2.0f * 3.141592f) * (float)i / numStars;
		angle += worldTime * 3.0f;
		moon->setLocation(vector3(0.0f, 25.0f * std::cos(angle), 25.0f * std::sin(angle)));
	}
}
