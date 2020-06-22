#include "lightning_effect.h"

#include "pathos\mesh\static_mesh_component.h"
#include "pathos\mesh\mesh.h"
#include "pathos\mesh\geometry_primitive.h"

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
}
