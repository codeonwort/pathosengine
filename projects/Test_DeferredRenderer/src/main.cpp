#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/render/atmosphere.h"
#include "pathos/input/input_manager.h"
using namespace pathos;

#include <thread>
#include <algorithm>

#define VISUALIZE_CSM_FRUSTUM 0

#if VISUALIZE_CSM_FRUSTUM
#include "pathos/mesh/geometry_procedural.h"
#endif

const int32         WINDOW_WIDTH        =   1920;
const int32         WINDOW_HEIGHT       =   1080;
const bool          WINDOW_FULLSCREEN   =   false;
const char*         WINDOW_TITLE        =   "Test: Deferred Rendering";
const float         FOVY                =   60.0f;
const glm::vec3     CAMERA_POSITION     =   glm::vec3(0.0f, 25.0f, 200.0f);
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
	std::vector<Mesh*> balls;
#if VISUALIZE_CSM_FRUSTUM
	Mesh* csmDebugger;
#endif

void setupInput();
void setupCSMDebugger();
void setupScene();
void tick(float deltaSeconds);

OBJLoader houseLoader;
bool asyncLoadComplete = false;
void asyncLoadTask() {
	//bool loaded = houseLoader.load("models/small_colonial_house/houseSF.obj", "models/small_colonial_house/");
	bool loaded = houseLoader.load("models/fireplace_room/fireplace_room.obj", "models/fireplace_room/");
	CHECK(loaded);
	asyncLoadComplete = true;
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
	cam->move(CAMERA_POSITION);

	std::thread asyncLoadWorker(asyncLoadTask);

#if VISUALIZE_CSM_FRUSTUM
	setupCSMDebugger();
#endif
	setupScene();

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	asyncLoadWorker.join();

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

	InputManager* inputManager = gEngine->getInputSystem()->getDefaultInputManager();
	inputManager->bindAxis("moveForward", moveForward);
	inputManager->bindAxis("moveRight", moveRight);
	inputManager->bindAxis("rotateYaw", rotateYaw);
	inputManager->bindAxis("rotatePitch", rotatePitch);
}

void setupCSMDebugger()
{
#if VISUALIZE_CSM_FRUSTUM
	float aspect_ratio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
	Camera tempCamera(new PerspectiveLens(FOVY, aspect_ratio, CAMERA_Z_NEAR, CAMERA_Z_FAR));
	tempCamera.move(CAMERA_POSITION);

	csmDebugger = new Mesh;
	csmDebugger->castsShadow = false;
	scene.add(csmDebugger);

	constexpr uint32 numFrustum = 4;
	std::vector<glm::vec3> frustumPlanes;
	tempCamera.getFrustum(frustumPlanes, numFrustum);

	bool cascadeMasks[4] = { true, true, true, true };

	// Wireframe for camera frustum
	{
		ProceduralGeometry* G = new ProceduralGeometry;
		WireframeMaterial* M = new WireframeMaterial(1.0f, 1.0f, 1.0f);
		csmDebugger->add(G, M);

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
		ProceduralGeometry* G = new ProceduralGeometry;
		WireframeMaterial* M = new WireframeMaterial(1.0f, 0.0f, 0.0f);
		csmDebugger->add(G, M);

		auto calcBounds = [](const glm::vec3* frustum, std::vector<glm::vec3>& outVertices) -> void {
			glm::vec3 sun_origin(0.0f, 0.0f, 0.0f);
			glm::vec3 sun_direction = SUN_DIRECTION;
			glm::vec3 sun_up(0.0f, 1.0f, 0.0f);

			// if almost parallel, choose another random direction
			float angle = glm::dot(sun_up, sun_direction);
			if (fabs(angle) >= 0.999f) {
				sun_up = glm::vec3(1.0f, 0.0f, 0.0f);
			}

			glm::mat4 lightView = glm::lookAt(sun_origin, sun_direction, sun_up);
			glm::vec3 sun_right = glm::vec3(lightView[0][0], lightView[1][0], lightView[2][0]);
			sun_up = glm::vec3(lightView[0][1], lightView[1][1], lightView[2][1]);

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
#endif
}

void setupScene() {
	sunLight = new DirectionalLight(SUN_DIRECTION, glm::vec3(1.0f, 1.0f, 1.0f));
	scene.add(sunLight);

	scene.add(new PointLight(glm::vec3(-50.0f, 60.0f, 170.0f), 5.0f * glm::vec3(0.2f, 1.0f, 1.0f), 100.0f, 0.001f));
	scene.add(new PointLight(glm::vec3(0.0f, 30.0f, 150.0f), 5.0f * glm::vec3(1.0f, 0.2f, 1.0f), 100.0f, 0.001f));
	scene.add(new PointLight(glm::vec3(-20.0f, 50.0f, 50.0f), 2.0f * glm::vec3(1.0f, 0.0f, 0.0f), 80.0f, 0.001f));
	scene.add(new PointLight(glm::vec3(-20.0f, 50.0f, 150.0f), 1.0f * glm::vec3(1.0f, 1.0f, 1.0f), 500.0f, 0.0001f));

	//---------------------------------------------------------------------------------------
	// create materials
	//---------------------------------------------------------------------------------------
	const char* cubeImgName[6] = {
		"resources/cubemap1/pos_x.jpg", "resources/cubemap1/neg_x.jpg",
		"resources/cubemap1/pos_y.jpg", "resources/cubemap1/neg_y.jpg",
		"resources/cubemap1/pos_z.jpg", "resources/cubemap1/neg_z.jpg"
	};
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = pathos::loadImage(cubeImgName[i]);
	GLuint cubeTexture = pathos::loadCubemapTexture(cubeImg, true);

	GLuint tex = pathos::loadTexture(loadImage("resources/154.jpg"), true, true);
	GLuint tex_norm = pathos::loadTexture(loadImage("resources/154_norm.jpg"), true, false);

	auto material_texture = new TextureMaterial(tex);
	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(2.0f, 0.2f, 0.2f);
		color->setMetallic(0.2f);
		color->setRoughness(0.1f);
	}
	auto material_cubemap = new CubeEnvMapMaterial(cubeTexture);

	// PBR material
	PBRTextureMaterial* material_pbr;
	{
		constexpr bool sRGB = true;
#if 1
		GLuint albedo		= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-albedo.png"), true, sRGB);
		GLuint normal		= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-normal-ue.png"), true, !sRGB);
		GLuint metallic		= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-metalness.png"), true, !sRGB);
		GLuint roughness	= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-roughness.png"), true, !sRGB);
		GLuint ao			= pathos::loadTexture(loadImage("resources/pbr_sandstone/sandstonecliff-ao.png"), true, !sRGB);
#else
		GLuint albedo		= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-albedo.png"), true, sRGB);
		GLuint normal		= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-normal.png"), true, !sRGB);
		GLuint metallic		= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-metalness.png"), true, !sRGB);
		GLuint roughness	= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-rough.png"), true, !sRGB);
		GLuint ao			= pathos::loadTexture(loadImage("resources/pbr_redbricks/redbricks2b-ao.png"), true, !sRGB);
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

	for (auto i = 0u; i < NUM_BALLS; ++i) {
		Mesh* ball = new Mesh(geom_sphere, material_pbr);
		ball->getTransform().setScale(2.0f + (float)i * 0.1f);
		ball->getTransform().setLocation(-200.0f + (float)i * 5.0f, -15.0f, 50.0f -30.0f * i);
		balls.push_back(ball);
		scene.add(ball);
	}

	godRaySource = new Mesh(geom_sphere, material_color);
	godRaySource->getTransform().setScale(10.0f);
	godRaySource->getTransform().setLocation(0.0f, 300.0f, -500.0f);

	Skybox* skybox = new Skybox(cubeTexture);
	skybox->setLOD(1.0f);

	// add to scene
	scene.add(ground);
	scene.sky = skybox;
	//scene.sky = new AtmosphereScattering;
	scene.godRaySource = godRaySource;
}

void tick(float deltaSeconds)
{
	if(asyncLoadComplete) {
		asyncLoadComplete = false;

		Mesh* house = houseLoader.craftMeshFromAllShapes();
		house->getTransform().setRotation(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		house->getTransform().setScale(50.0f);
		house->getTransform().setLocation(-100.0f, -10.0f, 0.0f);

		scene.add(house);
		houseLoader.unload();
	}

	{
		InputManager* input = gEngine->getInputSystem()->getDefaultInputManager();

		// movement per seconds
		const float speedX = 400.0f * deltaSeconds;
		const float speedY = -200.0f * deltaSeconds;
		const float rotateY = 120.0f * deltaSeconds;
		const float rotateX = 120.0f * deltaSeconds;

		float dx   = input->getAxis("moveRight") * speedX;
		float dz   = input->getAxis("moveForward") * speedY;
		float rotY = input->getAxis("rotateYaw") * rotateY;
		float rotX = input->getAxis("rotatePitch") * rotateX;

		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}

	for (Mesh* ball : balls) {
		ball->getTransform().setRotation(0.005f, glm::vec3(0.0f, 1.0f, 1.0f));
	}

//#if !(_DEBUG)
//	char title[256];
//	sprintf_s(title, "%s (Elapsed: %.2f ms)", WINDOW_TITLE, gEngine->getMilliseconds());
//	glutSetWindowTitle(title); // #todo: why this terminates the application in debug build after switching glLoadGen to gl3w?
//#endif

}
