#include "world_lightroom_old.h"
#include "player_controller.h"

#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/material/material_shader.h"
#include "pathos/scene/static_mesh_actor.h"
#include "pathos/scene/point_light_actor.h"
#include "pathos/scene/directional_light_actor.h"
#include "pathos/scene/rect_light_actor.h"

#define TEST_POINT_LIGHT 1
#define TEST_RECT_LIGHT  1

// --------------------------------------------------------
// Constants

static const vector3 CAMERA_POSITION       = vector3(0.7f, 0.6f, 2.5f);
static const vector3 CAMERA_LOOK_AT        = vector3(0.0f, 0.1f, 0.0f);
static const vector3 SUN_DIRECTION         = glm::normalize(vector3(0.0f, -1.0f, -1.0f));
// No Sun
static const vector3 SUN_COLOR             = vector3(1.0f, 1.0f, 1.0f);
static const float   SUN_ILLUMINANCE       = 0.0f;
// Household Light Bulb : 2700K, 800 lm ( https://physicallybased.info/ )
static const vector3 POINT_LIGHT_COLOR     = vector3(255, 169, 87) / 255.0f;
static const float   POINT_LIGHT_INTENSITY = 800.0f / (3.14f * 4.0f);
static const vector3 RECT_LIGHT_COLOR      = vector3(255, 169, 87) / 255.0f;
static const float   RECT_LIGHT_INTENSITY  = 800.0f;

// --------------------------------------------------------
// World

void World_LightRoomOld::onInitialize() {
	getCamera().lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, vector3(0.0f, 1.0f, 0.0f));
	setupInput();
	setupScene();
}

void World_LightRoomOld::onTick(float deltaSeconds) {
	if (TEST_RECT_LIGHT) {
		float t = gEngine->getWorldTime() * 1.57f;
		float w = 0.01f * (25.0f + 10.0f * cosf(t));
		float h = 0.01f * (15.0f + 5.0f * sinf(t));
		rectLight0->setLightSize(w, h);
		rectLight0Gizmo->setActorScale(vector3(1.0f, h / 0.15f, w / 0.25f));
	}

	if (TEST_POINT_LIGHT) {
		float k = 0.5f * (1.0f + cosf(gEngine->getWorldTime()));
		float radii = 0.01f + k * (0.1f - 0.01f);
		pointLight0->setSourceRadius(radii);
		pointLight0Gizmo->setActorScale(radii);
	}
}

void World_LightRoomOld::setupInput() {
	playerController = spawnActor<PlayerController>();
}

void World_LightRoomOld::setupScene() {
	// --------------------------------------------------------
	// Ground & walls

	auto G_ground = makeAssetPtr<PlaneGeometry>(PlaneGeometry::Input{ 10.0f, 10.0f, 10, 10 });

#if 1
	auto M_ground = Material::createMaterialInstance("solid_color");
	M_ground->setConstantParameter("albedo", vector3(0.33f, 0.22f, 0.18f)); // brown
	M_ground->setConstantParameter("roughness", 0.2f);
	M_ground->setConstantParameter("metallic", 0.0f);
	M_ground->setConstantParameter("emissive", vector3(0.0f));
#else
	// unlit test
	auto M_ground = Material::createMaterialInstance("unlit");
	M_ground->setConstantParameter("color", vector3(0.0f, 0.0f, 0.5f));
#endif

	ground = spawnActor<StaticMeshActor>();
	ground->setStaticMesh(makeAssetPtr<StaticMesh>(G_ground, M_ground));
	ground->setActorLocation(0.0f, 0.0f, 0.0f);
	ground->setActorRotation(Rotator(0.0f, -90.0f, 0.0f));

	auto M_wall = Material::createMaterialInstance("solid_color");
	M_wall->setConstantParameter("albedo", vector3(0.5f, 0.9f, 0.5f));
	M_wall->setConstantParameter("roughness", 0.8f);
	M_wall->setConstantParameter("metallic", 0.0f);
	M_wall->setConstantParameter("emissive", vector3(0.0f));

	auto mesh_wall = makeAssetPtr<StaticMesh>(G_ground, M_wall);
	mesh_wall->doubleSided = true;

	wallA = spawnActor<StaticMeshActor>();
	wallA->setStaticMesh(mesh_wall);
	wallA->setActorLocation(0.0f, 0.0f, -0.3f);
	wallB = spawnActor<StaticMeshActor>();
	wallB->setStaticMesh(mesh_wall);
	wallB->setActorLocation(-0.3f, 0.0f, 0.0f);
	wallB->setActorRotation(Rotator(90.0f, 0.0f, 0.0f));

	// --------------------------------------------------------
	// Box

	const float boxHalfSize = 0.1f;

	auto G_box = makeAssetPtr<CubeGeometry>(vector3(boxHalfSize));
	auto G_ball = makeAssetPtr<SphereGeometry>(SphereGeometry::Input{ boxHalfSize });

	auto M_box = Material::createMaterialInstance("solid_color");
	M_box->setConstantParameter("albedo", vector3(0.9f, 0.9f, 0.9f));
	M_box->setConstantParameter("roughness", 0.2f);
	M_box->setConstantParameter("metallic", 0.0f);
	M_box->setConstantParameter("emissive", vector3(0.0f));

	auto M_ball = Material::createMaterialInstance("solid_color");
	M_ball->setConstantParameter("albedo", vector3(0.9f, 0.1f, 0.1f));
	M_ball->setConstantParameter("roughness", 0.3f);
	M_ball->setConstantParameter("metallic", 0.0f);
	M_ball->setConstantParameter("emissive", vector3(0.0f));

	box = spawnActor<StaticMeshActor>();
	box->setStaticMesh(makeAssetPtr<StaticMesh>(G_box, M_box));
	box->setActorLocation(0.0f, boxHalfSize, 0.5f);

	ball = spawnActor<StaticMeshActor>();
	ball->setStaticMesh(makeAssetPtr<StaticMesh>(G_ball, M_ball));
	ball->setActorLocation(1.0f, boxHalfSize, 0.5f);

	// --------------------------------------------------------
	// Lights

	sun = spawnActor<DirectionalLightActor>();
	sun->setDirection(SUN_DIRECTION);
	sun->setColorAndIlluminance(SUN_COLOR, SUN_ILLUMINANCE);

#if TEST_POINT_LIGHT
	pointLight0 = spawnActor<PointLightActor>();
	pointLight0->setActorLocation(1.0f + boxHalfSize * 1.5f, boxHalfSize * 3.0f, 0.0f);
	pointLight0->setColorAndIntensity(POINT_LIGHT_COLOR, POINT_LIGHT_INTENSITY);
	pointLight0->setAttenuationRadius(0.7f);

	auto G_pointLightGizmo = makeAssetPtr<SphereGeometry>(SphereGeometry::Input{ 1.0f });
	auto M_pointLightGizmo = Material::createMaterialInstance("solid_color");
	M_pointLightGizmo->setConstantParameter("albedo", vector3(0.0f, 0.0f, 0.0f));
	M_pointLightGizmo->setConstantParameter("metallic", 0.0f);
	M_pointLightGizmo->setConstantParameter("roughness", 0.0f);
	vector3 plGizmoEm = 10.0f * pointLight0->getLightComponent()->color;
	M_pointLightGizmo->setConstantParameter("emissive", vector3(plGizmoEm.x, plGizmoEm.y, plGizmoEm.z));
	pointLight0Gizmo = spawnActor<StaticMeshActor>();
	pointLight0Gizmo->setStaticMesh(makeAssetPtr<StaticMesh>(G_pointLightGizmo, M_pointLightGizmo));
	pointLight0Gizmo->setActorLocation(pointLight0->getActorLocation());
#endif

#if TEST_RECT_LIGHT
	rectLight0 = spawnActor<RectLightActor>();
	rectLight0->setActorLocation(boxHalfSize * 1.5f, boxHalfSize * 2.5f, -0.1f);
	rectLight0->setActorRotation(Rotator(-120.0f, 0.0f, -20.0f));
	rectLight0->setLightSize(0.25f, 0.15f);
	rectLight0->setColorAndIntensity(RECT_LIGHT_COLOR, RECT_LIGHT_INTENSITY);
	rectLight0->setAttenuationRadius(3.0f);

	PlaneGeometry::Input rectLightGizmoInput{
		rectLight0->getLightComponent()->width,
		rectLight0->getLightComponent()->height,
		1, 1, PlaneGeometry::Direction::X
	};
	auto G_rectLightGizmo = makeAssetPtr<PlaneGeometry>(rectLightGizmoInput);

	auto M_rectLightGizmo = Material::createMaterialInstance("solid_color");
	M_rectLightGizmo->copyParametersFrom(M_pointLightGizmo.get());
	vector3 rectGizmoEm = 10.0f * rectLight0->getLightComponent()->color;
	M_rectLightGizmo->setConstantParameter("emissive", vector3(rectGizmoEm.x, rectGizmoEm.y, rectGizmoEm.z));
	rectLight0Gizmo = spawnActor<StaticMeshActor>();
	rectLight0Gizmo->setStaticMesh(makeAssetPtr<StaticMesh>(G_rectLightGizmo, M_rectLightGizmo));
	rectLight0Gizmo->getStaticMeshComponent()->castsShadow = false;
	vector3 rectForward = rectLight0->getActorRotation().toDirection();
	rectLight0Gizmo->setActorLocation(rectLight0->getActorLocation() - (0.01f * rectForward));
	rectLight0Gizmo->setActorRotation(rectLight0->getActorRotation());
#endif
}
