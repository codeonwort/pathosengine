#include "lightning_effect.h"

#include "badger/math/random.h"

#include "pathos/util/math_lib.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/static_mesh_component.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/geometry_procedural.h"

static const vector3 LIGHTNING_PARTICLE_EMISSIVE(2.0f, 2.0f, 2.0f);

LightningActor::LightningActor()
{
	sphereComponent = createDefaultComponent<StaticMeshComponent>();

	sphereGeometry = new SphereGeometry(1.0f, 50);
	sphereMaterial = new ColorMaterial;
	sphereMaterial->setAlbedo(0.2f, 0.3f, 0.8f);
	sphereMaterial->setRoughness(0.0f);
	sphereMaterial->setMetallic(0.0f);
	sphereMaterial->setEmissive(3.0f, 3.0f, 3.0f);
	sphereComponent->setStaticMesh(new Mesh(sphereGeometry, sphereMaterial));

	setAsRootComponent(sphereComponent);
}

void LightningActor::generateParticle(const vector3& p0, const vector3& p1)
{
	LightningParticleComponent* component = new LightningParticleComponent;
	registerComponent(component);

	particleComponents.push_back(component);
	component->generateParticle(p0, p1);
}

LightningParticleComponent::LightningParticleComponent()
{
	G = new ProceduralGeometry;

	M = new ColorMaterial;
	M->setAlbedo(0.0f, 0.0f, 0.0f);
	M->setRoughness(0.0f);
	M->setMetallic(0.0f);
	M->setEmissive(LIGHTNING_PARTICLE_EMISSIVE.x, LIGHTNING_PARTICLE_EMISSIVE.y, LIGHTNING_PARTICLE_EMISSIVE.z);
	M->billboard = true;
	M->billboardWidth = 5.0f;

	setStaticMesh(new Mesh(G, M));
	getStaticMesh()->doubleSided = true;
}

void LightningParticleComponent::generateParticle(const vector3& startPosition, const vector3& endPosition)
{
	G->clear();

	std::vector<vector3> positions;
	std::vector<vector2> uvs;
	std::vector<uint32> indices;

	const uint32 numSubdivisions = 4;
	const float jitter = glm::length(endPosition - startPosition) * 0.02f;
	
	auto recurse = [&](const vector3& p0, const vector3& p1, uint32 depth) -> void {
		auto recurse_impl = [&](const vector3& p0, const vector3& p1, uint32 depth, auto& impl) -> void {
			vector3 middle = (0.5f + ((Random() - 0.5f) * 0.2f)) * (p0 + p1);
			vector3 T, B;
			pathos::calculateOrthonormalBasis(glm::normalize(p1 - p0), T, B);
			float jitter_scaled = jitter * (1.0f - (float)depth / numSubdivisions);
			T = jitter * Random() * T;
			B = jitter * Random() * B;
			middle += T + B;

			if (depth > 0) {
				impl(p0, middle, depth - 1, impl);
			}
			positions.push_back(middle);
			positions.push_back(middle);
			uvs.push_back(vector2(0.0f, 0.0f));
			uvs.push_back(vector2(1.0f, 0.0f));
			if (depth > 0) {
				impl(middle, p1, depth - 1, impl);
			}
		};
		recurse_impl(p0, p1, depth, recurse_impl);
	};

	positions.push_back(startPosition);
	positions.push_back(startPosition);
	uvs.push_back(vector2(0.0f, 0.0f));
	uvs.push_back(vector2(1.0f, 0.0f));
	recurse(startPosition, endPosition, numSubdivisions);
	positions.push_back(endPosition);
	positions.push_back(endPosition);
	uvs.push_back(vector2(0.0f, 0.0f));
	uvs.push_back(vector2(1.0f, 0.0f));

	uint32 numVertices = (uint32)positions.size() / 2;
	for (uint32 i = 0; i < numVertices - 1; ++i) {
		indices.push_back(i * 2 + 0); indices.push_back(i * 2 + 1); indices.push_back(i * 2 + 2);
		indices.push_back(i * 2 + 1); indices.push_back(i * 2 + 3); indices.push_back(i * 2 + 2);
	}
	
	G->updatePositionData((float*)positions.data(), (uint32)(positions.size() * 3));
	G->updateUVData((float*)uvs.data(), (uint32)(uvs.size() * 2));
	G->updateIndexData(indices.data(), (uint32)indices.size());
	G->calculateNormals();
	G->calculateTangentBasis();
}
