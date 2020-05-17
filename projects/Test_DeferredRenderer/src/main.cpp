#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/render/atmosphere.h"
#include "pathos/input/input_manager.h"
#include "pathos/loader/asset_streamer.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/gui/gui_window.h"
#include "pathos/util/math_lib.h"
using namespace pathos;

#include "point_light_actor.h"

#define VISUALIZE_CSM_FRUSTUM 0
#define DEBUG_SKYBOX          0

#if VISUALIZE_CSM_FRUSTUM
#include "pathos/mesh/geometry_procedural.h"
#endif

const int32         WINDOW_WIDTH        =   1920;
const int32         WINDOW_HEIGHT       =   1080;
const bool          WINDOW_FULLSCREEN   =   false;
const char*         WINDOW_TITLE        =   "Test: Deferred Rendering";
const float         FOVY                =   60.0f;
const glm::vec3     CAMERA_POSITION     =   glm::vec3(20.0f, 25.0f, 200.0f);
const glm::vec3     CAMERA_LOOK_AT      =   glm::vec3(20.0f, 25.0f, 190.0f);
const float         CAMERA_Z_NEAR       =   1.0f;
const float         CAMERA_Z_FAR        =   5000.0f;
const glm::vec3     SUN_DIRECTION       =   glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
const uint32        NUM_BALLS           =   10;

// World
Camera* cam;
Scene scene;
	DirectionalLight* sunLight;
	Mesh* godRaySource;
	Mesh* ground;
	Mesh* objModel;
	std::vector<Mesh*> balls;
	std::vector<Mesh*> boxes;
#if VISUALIZE_CSM_FRUSTUM
	Mesh* csmDebugger;
#endif

void setupInput();
void setupCSMDebugger();
void setupScene(); // #todo-actor: Remove this
void setupSceneWithActor(Scene* scene); // #todo-actor: Port everything from setupScene()

void tick(float deltaSeconds);

void onLoadWavefrontOBJ(OBJLoader* loader) {
	objModel = loader->craftMeshFromAllShapes();
	objModel->getTransform().setRotation(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	objModel->getTransform().setScale(50.0f);
	objModel->getTransform().setLocation(-100.0f, -10.0f, 0.0f);

	for(Material* M : objModel->getMaterials()) {
		ColorMaterial* CM = dynamic_cast<ColorMaterial*>(M);
		if(CM) {
			CM->setRoughness(1.0f);
		}
	}

	scene.add(objModel);
}

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.fullscreen   = WINDOW_FULLSCREEN;
	conf.title        = WINDOW_TITLE;
	conf.rendererType = ERendererType::Deferred;
	conf.tick         = tick;
	Engine::init(argc, argv, conf);

	setupInput();

	// camera
	float aspect_ratio = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOVY, aspect_ratio, CAMERA_Z_NEAR, CAMERA_Z_FAR));
	cam->lookAt(CAMERA_POSITION, CAMERA_LOOK_AT, glm::vec3(0.0f, 1.0f, 0.0f));

	gEngine->getAssetStreamer()->enqueueWavefrontOBJ("models/fireplace_room/fireplace_room.obj", "models/fireplace_room/", onLoadWavefrontOBJ);

#if VISUALIZE_CSM_FRUSTUM
	setupCSMDebugger();
#endif
	setupScene();
	setupSceneWithActor(&scene);

	gEngine->setWorld(&scene, cam);

	gEngine->start();

	return 0;
}

void setupInput()
{
	AxisBinding moveForward;
	moveForward.addInput(InputConstants::KEYBOARD_W, 1.0f);
	moveForward.addInput(InputConstants::KEYBOARD_S, -1.0f);

	AxisBinding moveRight;
	moveRight.addInput(InputConstants::KEYBOARD_D, 1.0f);
	moveRight.addInput(InputConstants::KEYBOARD_A, -1.0f);

	AxisBinding rotateYaw;
	rotateYaw.addInput(InputConstants::KEYBOARD_Q, -1.0f);
	rotateYaw.addInput(InputConstants::KEYBOARD_E, 1.0f);

	AxisBinding rotatePitch;
	rotatePitch.addInput(InputConstants::KEYBOARD_Z, -1.0f);
	rotatePitch.addInput(InputConstants::KEYBOARD_X, 1.0f);

	AxisBinding moveFast;
	moveFast.addInput(InputConstants::SHIFT, 1.0f);

	ButtonBinding drawShadowFrustum;
	drawShadowFrustum.addInput(InputConstants::KEYBOARD_F);

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindAxis("moveForward", moveForward);
	inputManager->bindAxis("moveRight", moveRight);
	inputManager->bindAxis("rotateYaw", rotateYaw);
	inputManager->bindAxis("rotatePitch", rotatePitch);
	inputManager->bindAxis("moveFast", moveFast);
	inputManager->bindButtonPressed("drawShadowFrustum", drawShadowFrustum, setupCSMDebugger);
}

void setupCSMDebugger()
{
#if VISUALIZE_CSM_FRUSTUM
	static bool firstRun = true;

	float aspect_ratio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
	Camera tempCamera(new PerspectiveLens(FOVY, aspect_ratio, CAMERA_Z_NEAR, CAMERA_Z_FAR));
	tempCamera.lookAt(cam->getPosition(), cam->getPosition() + cam->getEyeVector(), glm::vec3(0.0f, 1.0f, 0.0f));

	if (firstRun) {
		csmDebugger = new Mesh;
		csmDebugger->castsShadow = false;
		scene.add(csmDebugger);
	}

	constexpr uint32 numFrustum = 4;
	std::vector<glm::vec3> frustumPlanes;
	tempCamera.getFrustum(frustumPlanes, numFrustum);

	bool cascadeMasks[4] = { true, true, true, true };

	// Wireframe for camera frustum
	{
		static ProceduralGeometry* G = new ProceduralGeometry;
		static WireframeMaterial* M = new WireframeMaterial(1.0f, 1.0f, 1.0f);
		if (firstRun) {
			csmDebugger->add(G, M);
		}
		G->clear();

		constexpr uint32 iMax = 4 * (numFrustum + 1);
		for (uint32 i = 0; i < iMax; i += 4) {
			if (cascadeMasks[i / 4] == false || (i/4 == 4 && cascadeMasks[3] == false)) {
				continue;
			}

			glm::vec3 p0 = frustumPlanes[i + 0];
			glm::vec3 p1 = frustumPlanes[i + 1];
			glm::vec3 p2 = frustumPlanes[i + 2];
			glm::vec3 p3 = frustumPlanes[i + 3];
			G->addTriangle(p0, p1, p2);
			G->addTriangle(p1, p2, p3);

			if (i < iMax - 4 && (i/4 != 4 || cascadeMasks[i/4 - 1])) {
				glm::vec3 p4 = frustumPlanes[i + 4];
				glm::vec3 p5 = frustumPlanes[i + 5];
				glm::vec3 p6 = frustumPlanes[i + 6];
				glm::vec3 p7 = frustumPlanes[i + 7];
				G->addQuad(p0, p4, p5, p1);
				G->addQuad(p2, p6, p7, p3);
				G->addQuad(p0, p2, p6, p4);
				G->addQuad(p1, p3, p7, p5);
			}
		}
		G->upload();
		G->calculateNormals();
		G->calculateTangentBasis();
	}

	// Wireframe for bounds of light view projections
	{
		static ProceduralGeometry* G = new ProceduralGeometry;
		static WireframeMaterial* M = new WireframeMaterial(1.0f, 0.0f, 0.0f);
		if (firstRun) {
			csmDebugger->add(G, M);
		}
		G->clear();

		auto calcBounds = [](const glm::vec3* frustum, std::vector<glm::vec3>& outVertices) -> void {
			glm::vec3 sun_direction = SUN_DIRECTION;
			glm::vec3 sun_up, sun_right;
			pathos::calculateOrthonormalBasis(sun_direction, sun_up, sun_right);

			glm::vec3 frustum_center(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				frustum_center += frustum[i];
			}
			frustum_center *= 0.125f;

			glm::vec3 frustum_size(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				glm::vec3 delta = frustum[i] - frustum_center;
				frustum_size.x = std::max(frustum_size.x, fabs(glm::dot(delta, sun_right)));
				frustum_size.y = std::max(frustum_size.y, fabs(glm::dot(delta, sun_up)));
				frustum_size.z = std::max(frustum_size.z, fabs(glm::dot(delta, sun_direction)));
			}

			const glm::vec3 signs[8] = {
				glm::vec3(1,1,1), glm::vec3(1,1,-1), glm::vec3(1,-1,-1), glm::vec3(1,-1,1),
				glm::vec3(-1,1,1), glm::vec3(-1,1,-1), glm::vec3(-1,-1,-1), glm::vec3(-1,-1,1)
			};
			for (int32 i = 0; i < 8; ++i) {
				glm::vec3 s = signs[i] * frustum_size;
				glm::vec3 d = (s.x * sun_right) + (s.y * sun_up) + (s.z * sun_direction);
				glm::vec3 v = frustum_center + d;
				outVertices.push_back(v);
			}
		};

		std::vector<glm::vec3> lightViewVertices;
		for (uint32 i = 0u; i <= numFrustum; ++i) {
			calcBounds(&frustumPlanes[i * 4], lightViewVertices);
		}
		for (uint32 i = 0; i < (uint32)lightViewVertices.size(); i += 8) {
			if (cascadeMasks[i / 8] == false) {
				continue;
			}

			glm::vec3 p0 = lightViewVertices[i + 0];
			glm::vec3 p1 = lightViewVertices[i + 1];
			glm::vec3 p2 = lightViewVertices[i + 2];
			glm::vec3 p3 = lightViewVertices[i + 3];
			glm::vec3 p4 = lightViewVertices[i + 4];
			glm::vec3 p5 = lightViewVertices[i + 5];
			glm::vec3 p6 = lightViewVertices[i + 6];
			glm::vec3 p7 = lightViewVertices[i + 7];
			G->addQuad(p0, p1, p2, p3);
			G->addQuad(p4, p5, p6, p7);
			G->addQuad(p0, p3, p7, p4);
			G->addQuad(p0, p1, p5, p4);
			G->addQuad(p2, p3, p7, p6);
			G->addQuad(p1, p2, p6, p5);
		}
		G->upload();
		G->calculateNormals();
		G->calculateTangentBasis();
	}
	firstRun = false;
#endif
}

void setupScene() {
	//---------------------------------------------------------------------------------------
	// lighting
	//---------------------------------------------------------------------------------------
	sunLight = new DirectionalLight(SUN_DIRECTION, glm::vec3(1.0f, 1.0f, 1.0f));
	scene.add(sunLight);

	{
		GLuint equirectangularMap = pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr"));
		GLuint cubemapForIBL = IrradianceBaker::bakeCubemap(equirectangularMap, 512);

		// diffuse irradiance
		{
			GLuint irradianceMap = IrradianceBaker::bakeIrradianceMap(cubemapForIBL, 32, false);
			glObjectLabel(GL_TEXTURE, irradianceMap, -1, "diffuse irradiance");
			scene.irradianceMap = irradianceMap;
		}

		// specular IBL
		{
			GLuint prefilteredEnvMap;
			uint32 mipLevels;
			IrradianceBaker::bakePrefilteredEnvMap(cubemapForIBL, 128, prefilteredEnvMap, mipLevels);
			glObjectLabel(GL_TEXTURE, prefilteredEnvMap, -1, "specular IBL (prefiltered env map)");

			scene.prefilterEnvMap = prefilteredEnvMap;
			scene.prefilterEnvMapMipLevels = mipLevels;
		}
	}

	//---------------------------------------------------------------------------------------
	// create materials
	//---------------------------------------------------------------------------------------
#if DEBUG_SKYBOX
	std::array<const char*,6> cubeImgName = {
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
		constexpr bool sRGB = true;
#if 1
		GLuint albedo		= pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-albedo.png"), true, sRGB);
		GLuint normal		= pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-normal-ue.png"), true, !sRGB);
		GLuint metallic		= pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-metalness.png"), true, !sRGB);
		GLuint roughness	= pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-roughness.png"), true, !sRGB);
		GLuint ao			= pathos::createTextureFromBitmap(loadImage("resources/pbr_sandstone/sandstonecliff-ao.png"), true, !sRGB);
#else
		GLuint albedo		= pathos::createTextureFromBitmap(loadImage("resources/pbr_redbricks/redbricks2b-albedo.png"), true, sRGB);
		GLuint normal		= pathos::createTextureFromBitmap(loadImage("resources/pbr_redbricks/redbricks2b-normal.png"), true, !sRGB);
		GLuint metallic		= pathos::createTextureFromBitmap(loadImage("resources/pbr_redbricks/redbricks2b-metalness.png"), true, !sRGB);
		GLuint roughness	= pathos::createTextureFromBitmap(loadImage("resources/pbr_redbricks/redbricks2b-rough.png"), true, !sRGB);
		GLuint ao			= pathos::createTextureFromBitmap(loadImage("resources/pbr_redbricks/redbricks2b-ao.png"), true, !sRGB);
#endif
		material_pbr = new PBRTextureMaterial(albedo, normal, metallic, roughness, ao);
	}
	

	//---------------------------------------------------------------------------------------
	// create geometries
	//---------------------------------------------------------------------------------------
	
	auto geom_sphere_big	= new SphereGeometry(15.0f, 30);
	auto geom_sphere		= new SphereGeometry(5.0f, 30);
	auto geom_plane			= new PlaneGeometry(10.0f, 10.0f);
	auto geom_plane_big		= new PlaneGeometry(10.0f, 10.0f, 20, 20);
	auto geom_cube			= new CubeGeometry(glm::vec3(5.0f));

	geom_sphere->calculateTangentBasis();
	geom_sphere_big->calculateTangentBasis();
	geom_plane->calculateTangentBasis();
	geom_plane_big->calculateTangentBasis();
	geom_cube->calculateTangentBasis();


	//---------------------------------------------------------------------------------------
	// create meshes
	//---------------------------------------------------------------------------------------

	ground = new Mesh(geom_plane_big, material_texture);
	ground->getTransform().setScale(1000.0f);
	ground->getTransform().setRotation(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ground->getTransform().setLocation(0.0f, -30.0f, 0.0f);
	ground->castsShadow = false;
	scene.add(ground);

	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		Mesh* ball = new Mesh(geom_sphere, material_pbr);
		ball->getTransform().setScale(5.0f + (float)i * 0.5f);
		ball->getTransform().setLocation(-400.0f, 50.0f, 300.0f - 100.0f * i);
		balls.push_back(ball);
		scene.add(ball);
	}
	for (uint32 i = 0u; i < NUM_BALLS; ++i) {
		ColorMaterial* ball_material = new ColorMaterial;
		ball_material->setAlbedo(0.5f, 0.3f, 0.3f);
		ball_material->setMetallic(0.2f);
		ball_material->setRoughness((float)i / NUM_BALLS);

		Mesh* ball = new Mesh(geom_cube, ball_material);
		ball->getTransform().setScale(5.0f + (float)i * 0.5f);
		ball->getTransform().setLocation(-550.0f, 50.0f, 300.0f - 100.0f * i);
		balls.push_back(ball);
		scene.add(ball);
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

			Mesh* box = new Mesh(geom_cube, box_material);
			box->getTransform().setLocation(box_x0 + i * box_spaceX, 50.0f, box_y0 + j * box_spaceY);
			box->getTransform().setScale(glm::vec3(1.0f, 10.0f * 0.5f * (1.0f + wave), 1.0f));
			scene.add(box);

			boxes.push_back(box);
		}
	}

	godRaySource = new Mesh(geom_sphere, material_color);
	godRaySource->getTransform().setScale(10.0f);
	godRaySource->getTransform().setLocation(0.0f, 300.0f, -500.0f);
	scene.godRaySource = godRaySource;

	//---------------------------------------------------------------------------------------
	// sky
	//---------------------------------------------------------------------------------------
	//Skybox* skybox = new Skybox(cubeTexture);
	//skybox->setLOD(1.0f);
	//scene.sky = skybox;

	//scene.sky = new AtmosphereScattering;

	//scene.sky = new AnselSkyRendering(pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr")));

	GLuint hdri_temp = pathos::createTextureFromHDRImage(pathos::loadHDRImage("resources/HDRI/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr"));
	scene.sky = new Skybox(IrradianceBaker::bakeCubemap(hdri_temp, 512));
}

void setupSceneWithActor(Scene* scene) {
	//////////////////////////////////////////////////////////////////////////
	// Test
	class TestActor : public Actor {
	public:
		TestActor() {
			LOG(LogDebug, "[TestActor] ctor");
		}
		virtual void onTick(float deltaSeconds) override {
			LOG(LogDebug, "[TestActor] tick");
			if (cnt++ > 10) destroy();
		}
		virtual void onDestroy() override {
			LOG(LogDebug, "[TestActor] onDestroy");
		}
	private:
		int32 cnt = 0;
	};

	scene->spawnActor<TestActor>();

	//////////////////////////////////////////////////////////////////////////
	// Lighting
	PointLightActor* pointLight0 = scene->spawnActor<PointLightActor>();
	PointLightActor* pointLight1 = scene->spawnActor<PointLightActor>();
	PointLightActor* pointLight2 = scene->spawnActor<PointLightActor>();
	PointLightActor* pointLight3 = scene->spawnActor<PointLightActor>();

	pointLight0->setParameters(glm::vec3(-50.0f, 60.0f, 170.0f), 5.0f * glm::vec3(0.2f, 2.0f, 1.0f), 100.0f, 0.001f);
	pointLight1->setParameters(glm::vec3(0.0f, 30.0f, 150.0f), 5.0f * glm::vec3(2.0f, 0.2f, 1.0f), 100.0f, 0.001f);
	pointLight2->setParameters(glm::vec3(-20.0f, 50.0f, 50.0f), 2.0f * glm::vec3(2.0f, 0.0f, 0.0f), 80.0f, 0.001f);
	pointLight3->setParameters(glm::vec3(-20.0f, 50.0f, 150.0f), 1.0f * glm::vec3(2.0f, 2.0f, 2.0f), 500.0f, 0.0001f);
}

void tick(float deltaSeconds)
{
	{
		InputManager* input = gEngine->getInputSystem()->getDefaultInputManager();

		// movement per seconds
		const float moveMultiplier = pathos::max(1.0f, input->getAxis("moveFast") * 10.0f);
		const float speedRight = 400.0f * deltaSeconds * moveMultiplier;
		const float speedForward = 200.0f * deltaSeconds * moveMultiplier;
		const float rotateY = 120.0f * deltaSeconds;
		const float rotateX = 120.0f * deltaSeconds;

		float deltaRight   = input->getAxis("moveRight") * speedRight;
		float deltaForward = input->getAxis("moveForward") * speedForward;
		float rotY         = input->getAxis("rotateYaw") * rotateY;
		float rotX         = input->getAxis("rotatePitch") * rotateX;

		cam->moveForward(deltaForward);
		cam->moveRight(deltaRight);
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}

	for (Mesh* ball : balls) {
		ball->getTransform().setRotation(0.005f, glm::vec3(0.0f, 1.0f, 1.0f));
	}

	static float sinT = 0.0f;
	sinT += 6.28f * 0.25f * deltaSeconds;
	for (uint32 i = 0; i < 16; ++i)
	{
		for (uint32 j = 0; j < 16; ++j)
		{
			float wave = ::sinf(sinT + 13.2754f * (i*16+j)/256.0f + (6.3f * j/16.0f));
			boxes[i*16+j]->getTransform().setScale(glm::vec3(1.0f, 10.0f * 0.5f * (1.0f + wave), 1.0f));
		}
	}

#if 0 // lookat debug
	static float lookAtTime = 0.0f;
	lookAtTime += 6.28f * deltaSeconds * 0.1f;
	glm::vec3 lookDir(100.0f * sinf(lookAtTime), 0.0f, 100.0f * cosf(lookAtTime));
	cam->lookAt(CAMERA_POSITION, CAMERA_POSITION + lookDir, glm::vec3(0.0f, 1.0f, 0.0f));
#endif

	{
		char title[256];
		sprintf_s(title, "%s (GPU Time: %.2f ms)", WINDOW_TITLE, gEngine->getGPUTime());
		gEngine->getMainWindow()->setTitle(title);
	}
}
