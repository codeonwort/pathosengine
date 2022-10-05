#include "world_sponza.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/shader/material_shader.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/loader/gltf_loader.h"
//#include "pathos/light/point_light_actor.h"
//#include "pathos/light/rect_light_actor.h"

#include "player_controller.h"

#if SHARED_PTR_ACTORS
	#define TEMP_SPAWN_ACTOR(T) sharedPtr<T>(spawnActor<T>())
#else
	#define TEMP_SPAWN_ACTOR(T) spawnActor<T>()
#endif

#define GLTF_TESTCASE 1

#if GLTF_TESTCASE == 0
	#define GLTF_FILENAME "intel_sponza/Main.1_Sponza/NewSponza_Main_glTF_002.gltf"
	#define GLTF_SCALE_MULT 1.0f
#elif GLTF_TESTCASE == 1
	#define GLTF_FILENAME "damaged_helmet/DamagedHelmet.gltf"
	#define GLTF_SCALE_MULT 100.0f
#endif

// --------------------------------------------------------
// Constants

static const vector3 CAMERA_POSITION    = vector3(70.0f, 60.0f, 250.0f);
static const vector3 CAMERA_LOOK_AT     = vector3(0.0f, 10.0f, 0.0f);
static const vector3 SUN_DIRECTION      = glm::normalize(vector3(0.0f, -1.0f, -1.0f));
static const vector3 SUN_RADIANCE       = 10.0f * vector3(1.0f, 1.0f, 1.0f);

// --------------------------------------------------------
// World

void World_Sponza::onInitialize() {
	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
	setupInput();
	setupScene();

	GLTFLoader loader;
	bool bLoaded = loader.loadASCII(GLTF_FILENAME);
	if (bLoaded) {
		for (size_t i = 0; i < loader.numModels(); ++i) {
			StaticMeshActor* sm = spawnActor<StaticMeshActor>();
			const GLTFModelDesc& desc = loader.getModel(i);
			sm->setStaticMesh(desc.mesh);
			sm->setActorLocation(desc.translation);
			sm->setActorScale(desc.scale * GLTF_SCALE_MULT);
		}
	}
}

void World_Sponza::onTick(float deltaSeconds) {
	//
}

void World_Sponza::setupInput() {
	playerController = TEMP_SPAWN_ACTOR(PlayerController);
}

void World_Sponza::setupScene() {
	// --------------------------------------------------------
	// Ground

	MeshGeometry* G_ground = new PlaneGeometry(1000.0f, 1000.0f, 10, 10);

	Material* M_ground = Material::createMaterialInstance("solid_color");
	M_ground->setConstantParameter("albedo", vector3(0.33f, 0.22f, 0.18f)); // brown
	M_ground->setConstantParameter("roughness", 0.9f);
	M_ground->setConstantParameter("metallic", 0.0f);
	M_ground->setConstantParameter("emissive", vector3(0.0f));

	ground = TEMP_SPAWN_ACTOR(StaticMeshActor);
	ground->setStaticMesh(new Mesh(G_ground, M_ground));
	ground->setActorLocation(0.0f, 0.0f, 0.0f);
	ground->setActorRotation(Rotator(0.0f, -90.0f, 0.0f));

	// --------------------------------------------------------
	// Lights

	sun = TEMP_SPAWN_ACTOR(DirectionalLightActor);
	sun->setLightParameters(SUN_DIRECTION, SUN_RADIANCE);
}
