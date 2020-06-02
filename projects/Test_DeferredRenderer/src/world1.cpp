#include "world1.h"

#include "pathos/render/irradiance_baker.h"
#include "pathos/render/sky_ansel.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/light/directional_light_actor.h"
#include "pathos/light/point_light_actor.h"
#include "pathos/render/render_target.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/mesh/static_mesh_actor.h"
#include "pathos/input/input_manager.h"
#include "pathos/util/math_lib.h"
#include "pathos/gui/gui_window.h"
#include "pathos/scene/scene_capture_component.h"
#include "pathos/material/material.h"

#define OBJ_FILE "models/fireplace_room/fireplace_room.obj"
#define OBJ_DIR  "models/fireplace_room/"


const vector3       CAMERA_POSITION      = vector3(20.0f, 25.0f, 200.0f);
const vector3       CAMERA_LOOK_AT       = vector3(20.0f, 25.0f, 190.0f);
const vector3       SUN_DIRECTION        = glm::normalize(vector3(0.0f, -1.0f, -1.0f));
const vector3       SUN_RADIANCE         = 1.2f * vector3(1.0f, 1.0f, 1.0f);
const uint32        NUM_BALLS            = 10;

//////////////////////////////////////////////////////////////////////////
// #todo-world: Support callback defined in a class
class World1* worldInstance = nullptr;
static void onLoadOBJStatic(OBJLoader* loader) {
	worldInstance->onLoadOBJ(loader);
}
static void setupCSMDebuggerStatic() {
	worldInstance->setupCSMDebugger();
}
//////////////////////////////////////////////////////////////////////////

World1::World1()
{
	worldInstance = this;
}

void World1::onInitialize()
{
	gEngine->getAssetStreamer()->enqueueWavefrontOBJ(OBJ_FILE, OBJ_DIR, onLoadOBJStatic);

	setupInput();
	setupSky();
	setupScene();
}

void World1::setupInput()
{
	AxisBinding moveForward;
	moveForward.addInput(InputConstants::KEYBOARD_W, 1.0f);
	moveForward.addInput(InputConstants::KEYBOARD_S, -1.0f);

	AxisBinding moveRight;
	moveRight.addInput(InputConstants::KEYBOARD_D, 1.0f);
	moveRight.addInput(InputConstants::KEYBOARD_A, -1.0f);

	AxisBinding moveUp;
	moveUp.addInput(InputConstants::KEYBOARD_E, 1.0f);
	moveUp.addInput(InputConstants::KEYBOARD_Q, -1.0f);

	AxisBinding moveFast;
	moveFast.addInput(InputConstants::SHIFT, 1.0f);

	ButtonBinding drawShadowFrustum;
	drawShadowFrustum.addInput(InputConstants::KEYBOARD_F);

	ButtonBinding updateSceneCapture;
	updateSceneCapture.addInput(InputConstants::KEYBOARD_G);

	ButtonBinding rmb;
	rmb.addInput(InputConstants::MOUSE_RIGHT_BUTTON);

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindAxis("moveForward", moveForward);
	inputManager->bindAxis("moveRight", moveRight);
	inputManager->bindAxis("moveUp", moveUp);
	inputManager->bindAxis("moveFast", moveFast);
	inputManager->bindButtonPressed("drawShadowFrustum", drawShadowFrustum, setupCSMDebuggerStatic);
	inputManager->bindButtonPressed("updateSceneCapture", updateSceneCapture, [this]()
		{
			if (sceneCaptureComponent != nullptr) {
				sceneCaptureComponent->setLocation(camera.getPosition());
				sceneCaptureComponent->setRotation(Rotator(camera.getYaw(), camera.getPitch(), 0.0f));
				sceneCaptureComponent->captureScene();
			}
		}
	);
	inputManager->bindButtonPressed("RMB", rmb, [this]() { rotateByMouse = true; });
	inputManager->bindButtonReleased("RMB", rmb, [this]() { rotateByMouse = false; });
}

void World1::setupSky()
{
	{
		GLuint equirectangularMap = pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr"));
		GLuint cubemapForIBL = IrradianceBaker::bakeCubemap(equirectangularMap, 512);
		glObjectLabel(GL_TEXTURE, equirectangularMap, -1, "Texture IBL: equirectangularMap");
		glObjectLabel(GL_TEXTURE, cubemapForIBL, -1, "Texture IBL: cubemapForIBL");

		// diffuse irradiance
		{
			GLuint irradianceMap = IrradianceBaker::bakeIrradianceMap(cubemapForIBL, 32, false);
			glObjectLabel(GL_TEXTURE, irradianceMap, -1, "Texture IBL: diffuse irradiance");
			scene.irradianceMap = irradianceMap;
		}

		// specular IBL
		{
			GLuint prefilteredEnvMap;
			uint32 mipLevels;
			IrradianceBaker::bakePrefilteredEnvMap(cubemapForIBL, 128, prefilteredEnvMap, mipLevels);
			glObjectLabel(GL_TEXTURE, prefilteredEnvMap, -1, "Texture IBL: specular IBL (prefiltered env map)");

			scene.prefilterEnvMap = prefilteredEnvMap;
			scene.prefilterEnvMapMipLevels = mipLevels;
		}
	}

	//---------------------------------------------------------------------------------------
	// sky
	//---------------------------------------------------------------------------------------
#define SKY_METHOD 2

#if SKY_METHOD == 0
#if DEBUG_SKYBOX
	std::array<const char*, 6> cubeImgName = {
		"resources/placeholder/cubemap_right.jpg",
		"resources/placeholder/cubemap_left.jpg",
		"resources/placeholder/cubemap_top.jpg",
		"resources/placeholder/cubemap_bottom.jpg",
		"resources/placeholder/cubemap_front.jpg",
		"resources/placeholder/cubemap_back.jpg"
	};
#else
	std::array<const char*, 6> cubeImgName = {
		"resources/cubemap1/pos_x.jpg", "resources/cubemap1/neg_x.jpg",
		"resources/cubemap1/pos_y.jpg", "resources/cubemap1/neg_y.jpg",
		"resources/cubemap1/pos_z.jpg", "resources/cubemap1/neg_z.jpg"
	};
#endif
	std::array<FIBITMAP*, 6> cubeImg;
	pathos::loadCubemapImages(cubeImgName, ECubemapImagePreference::HLSL, cubeImg);
	GLuint cubeTexture = pathos::createCubemapTextureFromBitmap(cubeImg.data(), true);
	glObjectLabel(GL_TEXTURE, cubeTexture, -1, "skybox cubemap");

	Skybox* skybox = new Skybox(cubeTexture);
	skybox->setLOD(1.0f);
	scene.sky = skybox;
#elif SKY_METHOD == 1
	scene.sky = new AtmosphereScattering;
#elif SKY_METHOD == 2
	scene.sky = new AnselSkyRendering(pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr")));
#else
	GLuint hdri_temp = pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr"));
	scene.sky = new Skybox(IrradianceBaker::bakeCubemap(hdri_temp, 512));
#endif
}

void World1::setupScene()
{
	//---------------------------------------------------------------------------------------
	// create materials
	//---------------------------------------------------------------------------------------

	GLuint tex = pathos::createTextureFromBitmap(loadImage("resources/154.jpg"), true, true);
	GLuint tex_norm = pathos::createTextureFromBitmap(loadImage("resources/154_norm.jpg"), true, false);

	auto material_texture = new TextureMaterial(tex);
	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(2.0f, 0.2f, 0.2f);
		color->setMetallic(0.2f);
		color->setRoughness(0.1f);
	}

	// PBR material
	PBRTextureMaterial* material_pbr;
	{
		constexpr bool genMipmap = true;
		constexpr bool sRGB = true;
		GLuint albedo = pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-albedo.png"), genMipmap, sRGB);
		GLuint normal = pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-normal-ue.png"), genMipmap, !sRGB);
		GLuint metallic = pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-metalness.png"), genMipmap, !sRGB);
		GLuint roughness = pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-roughness.png"), genMipmap, !sRGB);
		GLuint ao = pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-ao.png"), genMipmap, !sRGB);

		material_pbr = new PBRTextureMaterial(albedo, normal, metallic, roughness, ao);
	}


	//---------------------------------------------------------------------------------------
	// create geometries
	//---------------------------------------------------------------------------------------

	auto geom_sphere_big = new SphereGeometry(15.0f, 30);
	auto geom_sphere = new SphereGeometry(5.0f, 30);
	auto geom_plane = new PlaneGeometry(10.0f, 10.0f);
	auto geom_plane_big = new PlaneGeometry(10.0f, 10.0f, 20, 20);
	auto geom_cube = new CubeGeometry(vector3(5.0f));

	geom_sphere->calculateTangentBasis();
	geom_sphere_big->calculateTangentBasis();
	geom_plane->calculateTangentBasis();
	geom_plane_big->calculateTangentBasis();
	geom_cube->calculateTangentBasis();

	//////////////////////////////////////////////////////////////////////////
	// Lighting
	DirectionalLightActor* dirLight = spawnActor<DirectionalLightActor>();
	dirLight->setLightParameters(SUN_DIRECTION, SUN_RADIANCE);

	PointLightActor* pointLight0 = spawnActor<PointLightActor>();
	PointLightActor* pointLight1 = spawnActor<PointLightActor>();
	PointLightActor* pointLight2 = spawnActor<PointLightActor>();
	PointLightActor* pointLight3 = spawnActor<PointLightActor>();

	pointLight0->setActorLocation(vector3(-50.0f, 60.0f, 170.0f));
	pointLight1->setActorLocation(vector3(0.0f, 30.0f, 150.0f));
	pointLight2->setActorLocation(vector3(-20.0f, 50.0f, 50.0f));
	pointLight3->setActorLocation(vector3(-20.0f, 50.0f, 150.0f));

	pointLight0->setLightParameters(5.0f * vector3(0.2f, 2.0f, 1.0f), 100.0f, 0.001f);
	pointLight1->setLightParameters(5.0f * vector3(2.0f, 0.2f, 1.0f), 100.0f, 0.001f);
	pointLight2->setLightParameters(2.0f * vector3(2.0f, 0.0f, 0.0f), 80.0f, 0.001f);
	pointLight3->setLightParameters(1.0f * vector3(2.0f, 2.0f, 2.0f), 500.0f, 0.0001f);

	godRaySource = spawnActor<StaticMeshActor>();
	godRaySource->setStaticMesh(new Mesh(geom_sphere, material_color));
	godRaySource->setActorScale(20.0f);
	godRaySource->setActorLocation(vector3(0.0f, 300.0f, -500.0f));
	godRaySource->getStaticMeshComponent()->castsShadow = false;
	getScene().godRaySource = godRaySource->getStaticMeshComponent();

	//////////////////////////////////////////////////////////////////////////
	// Static meshes

	ground = spawnActor<StaticMeshActor>();
	ground->setStaticMesh(new Mesh(geom_plane_big, material_texture));
	ground->setActorScale(1000.0f);
	ground->setActorRotation(Rotator(0.0f, -90.0f, 0.0f));
	ground->setActorLocation(vector3(0.0f, -30.0f, 0.0f));
	ground->getStaticMeshComponent()->castsShadow = false;

	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		StaticMeshActor* ball = spawnActor<StaticMeshActor>();
		ball->setStaticMesh(new Mesh(geom_sphere, material_pbr));
		ball->setActorScale(5.0f + (float)i * 0.5f);
		ball->setActorLocation(vector3(-400.0f, 50.0f, 300.0f - 100.0f * i));
		balls.push_back(ball);
	}
	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		ColorMaterial* ball_material = new ColorMaterial;
		ball_material->setAlbedo(0.5f, 0.3f, 0.3f);
		ball_material->setMetallic(0.2f);
		ball_material->setRoughness((float)i / NUM_BALLS);

		StaticMeshActor* ball = spawnActor<StaticMeshActor>();
		ball->setStaticMesh(new Mesh(geom_cube, ball_material));
		ball->setActorScale(5.0f + (float)i * 0.5f);
		ball->setActorLocation(vector3(-550.0f, 50.0f, 300.0f - 100.0f * i));
		balls.push_back(ball);
	}

	constexpr float box_x0 = 200.0f;
	constexpr float box_y0 = 60.0f;
	constexpr float box_spaceX = 20.0f;
	constexpr float box_spaceY = 20.0f;
	float sinT = 0.0f;
	ColorMaterial* box_material = new ColorMaterial;
	box_material->setAlbedo(1.0f, 1.0f, 1.0f);
	box_material->setMetallic(0.2f);
	box_material->setRoughness(0.5f);
	for (uint32 i = 0; i < 16; ++i)
	{
		for (uint32 j = 0; j < 16; ++j)
		{
			float wave = ::sinf(sinT += 0.0417f);

			StaticMeshActor* box = spawnActor<StaticMeshActor>();
			box->setStaticMesh(new Mesh(geom_cube, box_material));
			box->setActorLocation(vector3(box_x0 + i * box_spaceX, 50.0f, box_y0 + j * box_spaceY));
			box->setActorScale(vector3(1.0f, 10.0f * 0.5f * (1.0f + wave), 1.0f));

			boxes.push_back(box);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// scene capture test
	static RenderTarget2D* tempRenderTarget = nullptr;
	if (tempRenderTarget == nullptr) {
		tempRenderTarget = new RenderTarget2D;
		tempRenderTarget->respecTexture(1920, 1080, RenderTargetFormat::RGBA16F);
		tempRenderTarget->immediateUpdateResource();
	}

	static Actor* sceneCaptureActor = nullptr;
	if (sceneCaptureActor == nullptr) {
		sceneCaptureActor = spawnActor<Actor>();
		sceneCaptureComponent = new SceneCaptureComponent;
		sceneCaptureActor->registerComponent(sceneCaptureComponent);

		sceneCaptureComponent->renderTarget = tempRenderTarget;

		sceneCaptureActor->setActorLocation(CAMERA_POSITION);
	}

	sceneCaptureComponent->captureScene();

	auto material_sceneCapture = new TextureMaterial(tempRenderTarget->getGLName());
	StaticMeshActor* sceneCaptureViewer = spawnActor<StaticMeshActor>();
	sceneCaptureViewer->setStaticMesh(new Mesh(geom_plane, material_sceneCapture));
	sceneCaptureViewer->setActorLocation(-500.0f, 300.0f, -300.0f);
	sceneCaptureViewer->setActorScale(3.0f * vector3(16.0f, 9.0f, 1.0f));

	//////////////////////////////////////////////////////////////////////////
	// bloom test
	ColorMaterial* material_tooBright = new ColorMaterial;
	material_tooBright->setAlbedo(10.0f, 0.5f, 0.5f);
	material_tooBright->setMetallic(0.2f);
	material_tooBright->setRoughness(0.1f);
	StaticMeshActor* bloomActor = spawnActor<StaticMeshActor>();
	bloomActor->setActorLocation(200.0f, 80.0f, 600.0f);
	bloomActor->setActorScale(20.0f);
	bloomActor->setStaticMesh(new Mesh(geom_sphere, material_tooBright));
}

void World1::onLoadOBJ(OBJLoader* loader)
{
	objModel = spawnActor<StaticMeshActor>();
	objModel->setStaticMesh(loader->craftMeshFromAllShapes());
	objModel->setActorRotation(Rotator(-90.0f, 0.0f, 0.0f));
	objModel->setActorScale(50.0f);
	objModel->setActorLocation(vector3(-100.0f, -10.0f, 0.0f));

	for (Material* M : objModel->getStaticMesh()->getMaterials()) {
		ColorMaterial* CM = dynamic_cast<ColorMaterial*>(M);
		if (CM) {
			CM->setRoughness(1.0f);
		}
	}
}

void World1::setupCSMDebugger()
{
#if VISUALIZE_CSM_FRUSTUM
	static bool firstRun = true;
	if (firstRun) {
		csmDebugger = spawnActor<CSMDebugger>();
		firstRun = false;
	}

	csmDebugger->drawCameraFrustum(camera, SUN_DIRECTION);
#endif
}

void World1::onTick(float deltaSeconds)
{
	{
		static int32 prevMouseX = 0;
		static int32 prevMouseY = 0;

		InputManager* input = gEngine->getInputSystem()->getDefaultInputManager();

		int32 currMouseX = input->getMouseX();
		int32 currMouseY = input->getMouseY();

		// movement per seconds
		const float moveMultiplier = pathos::max(1.0f, input->getAxis("moveFast") * 10.0f);
		const float speedRight = 200.0f * deltaSeconds * moveMultiplier;
		const float speedForward = 200.0f * deltaSeconds * moveMultiplier;
		const float speedUp = 200.0f * deltaSeconds * moveMultiplier;
		const float rotateYaw = 120.0f * deltaSeconds;
		const float rotatePitch = 120.0f * deltaSeconds;

		float deltaRight = input->getAxis("moveRight") * speedRight;
		float deltaForward = input->getAxis("moveForward") * speedForward;
		float deltaUp = input->getAxis("moveUp") * speedUp;
		float rotY = 0.1f * (currMouseX - prevMouseX) * rotateYaw;
		float rotX = 0.1f * (currMouseY - prevMouseY) * rotatePitch;

		camera.moveForward(deltaForward);
		camera.moveRight(deltaRight);
		camera.moveUp(deltaUp);
		if (rotateByMouse) {
			camera.rotateYaw(rotY);
			camera.rotatePitch(rotX);
		}

		prevMouseX = currMouseX;
		prevMouseY = currMouseY;
	}

	static float ballAngle = 0.0f;
	for (StaticMeshActor* ball : balls) {
		Rotator rot = ball->getActorRotation();
		rot.yaw = fmod(rot.yaw + 1.0f, 180.0f);
		ball->setActorRotation(rot);
	}

	static float sinT = 0.0f;
	sinT += 6.28f * 0.25f * deltaSeconds;
	for (uint32 i = 0; i < 16; ++i)
	{
		for (uint32 j = 0; j < 16; ++j)
		{
			float wave = ::sinf(sinT + 13.2754f * (i * 16 + j) / 256.0f + (6.3f * j / 16.0f));
			boxes[i * 16 + j]->setActorScale(vector3(0.5f, 10.0f * 0.5f * (1.0f + wave), 0.5f));
		}
	}

#if 0 // lookat debug
	static float lookAtTime = 0.0f;
	lookAtTime += 6.28f * deltaSeconds * 0.1f;
	vector3 lookDir(100.0f * sinf(lookAtTime), 0.0f, 100.0f * cosf(lookAtTime));
	camera.lookAt(CAMERA_POSITION, CAMERA_POSITION + lookDir, vector3(0.0f, 1.0f, 0.0f));
#endif

	{
		char title[256];
		sprintf_s(title, "%s (CPU Time: %.2f ms, GPU Time: %.2f ms)",
			gEngine->getConfig().title, gEngine->getCPUTime(), gEngine->getGPUTime());
		gEngine->getMainWindow()->setTitle(title);
	}
}
