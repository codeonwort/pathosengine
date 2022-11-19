#include "lightning_effect.h"

#include "pathos/mesh/mesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/mesh/geometry_procedural.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/loader/imageloader.h"

#include "badger/math/vector_math.h"
#include "badger/math/random.h"

static const vector3 LIGHTNING_PARTICLE_EMISSIVE(10.0f, 10.0f, 25.0f);
static const float LIGHTNING_PARTICLE_THICKNESS = 80.0f;

#define LIGHTNING_MASK_TEXTURE  "resources/render_challenge_1/lightning_mask.jpg"
#define LIGHTNING_WARP_TEXTURE  "resources/render_challenge_1/lightning_warp.jpg"
#define BEND_LIHGTNING_GEOMETRY 1

// ------------------------------------------------------------
// LightningActor

LightningActor::LightningActor()
{
	sphereComponent = createDefaultComponent<StaticMeshComponent>();

	SphereGeometry* sphereGeometry = new SphereGeometry(1.0f, 50);
	Material* sphereMaterial = Material::createMaterialInstance("solid_color");
	sphereMaterial->setConstantParameter("albedo", vector3(0.2f, 0.3f, 0.8f));
	sphereMaterial->setConstantParameter("roughness", 0.0f);
	sphereMaterial->setConstantParameter("metallic", 0.0f);
	sphereMaterial->setConstantParameter("emissive", vector3(30.0f, 30.0f, 30.0f));
	sphereComponent->setStaticMesh(new Mesh(sphereGeometry, sphereMaterial));

	setAsRootComponent(sphereComponent);
}

void LightningActor::generateParticle(const vector3& p0, const vector3& p1)
{
	LightningParticleComponent* component = new LightningParticleComponent;
	registerComponent(component);

	particleComponents.push_back(component);
	component->setParameters(maskTexture, warpTexture);
	component->generateParticle(p0, p1);
}

void LightningActor::onSpawn()
{
	maskTexture = pathos::createTextureFromBitmap(
		pathos::loadImage(LIGHTNING_MASK_TEXTURE), false, false, "Tex_Lightning_Mask");
	warpTexture = pathos::createTextureFromBitmap(
		pathos::loadImage(LIGHTNING_WARP_TEXTURE), false, false, "Tex_Lightning_Mask");
}

void LightningActor::onDestroy()
{
	// #todo: Destroy maskTexture and warpTexture
}

// ------------------------------------------------------------
// LightningParticleComponent

LightningParticleComponent::LightningParticleComponent()
{
	G = new ProceduralGeometry;

	M = Material::createMaterialInstance("lightning_bolt");
	M->setConstantParameter("emissive", LIGHTNING_PARTICLE_EMISSIVE);
	M->setConstantParameter("billboardWidth", LIGHTNING_PARTICLE_THICKNESS);
	M->setConstantParameter("warpAngle", 15.0f + 30.0f * Random());

	setStaticMesh(new Mesh(G, M));
	getStaticMesh()->doubleSided = true;
}

void LightningParticleComponent::setParameters(GLuint maskTexture, GLuint warpTexture)
{
	M->setTextureParameter("maskTexture", maskTexture);
	M->setTextureParameter("warpTexture", warpTexture);
}

void LightningParticleComponent::generateParticle(const vector3& startPosition, const vector3& endPosition)
{
	G->clear();

	std::vector<vector3> positions;
	std::vector<vector2> uvs;
	std::vector<uint32> indices;

	const uint32 numSubdivisions = 4;
#if BEND_LIHGTNING_GEOMETRY
	const float jitter = glm::length(endPosition - startPosition) * 0.01f;
#else
	const float jitter = 0.0f;
#endif
	
	auto recurse = [&](const vector3& p0, const vector3& p1, uint32 depth) -> void {
		auto recurse_impl = [&](const vector3& p0, const vector3& p1, uint32 depth, auto& impl) -> void {
#if BEND_LIHGTNING_GEOMETRY
			vector3 middle = (0.5f + ((Random() - 0.5f) * 0.2f)) * (p0 + p1);
#else
			vector3 middle = 0.5f * (p0 + p1);
#endif
			vector3 T, B;
			badger::calculateOrthonormalBasis(glm::normalize(p1 - p0), T, B);
			float jitter_scaled = jitter * (1.0f - (float)depth / numSubdivisions);
			T = jitter * Random() * T;
			B = jitter * Random() * B;
			middle += T + B;

			if (depth > 0) {
				impl(p0, middle, depth - 1, impl);
			}
			positions.push_back(middle);
			positions.push_back(middle);
			float yRatio = glm::length(middle - startPosition) / glm::length(endPosition - startPosition);
			uvs.push_back(vector2(0.0f, yRatio));
			uvs.push_back(vector2(1.0f, yRatio));
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
	uvs.push_back(vector2(0.0f, 1.0f));
	uvs.push_back(vector2(1.0f, 1.0f));

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
