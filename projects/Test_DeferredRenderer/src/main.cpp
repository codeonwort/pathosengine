#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/render/atmosphere.h"
using namespace pathos;

#include <thread>


const int           WINDOW_WIDTH        =   1920;
const int           WINDOW_HEIGHT       =   1080;
const char*         WINDOW_TITLE        =   "Test: Deferred Rendering";
const float         FOVY                =   60.0f;
const glm::vec3     CAMERA_POSITION     =   glm::vec3(0.0f, 0.0f, 100.0f);
const float         CAMERA_Z_NEAR       =   1.0f;
const float         CAMERA_Z_FAR        =   2000.0f;
const glm::vec3     SUN_DIRECTION       =   glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
const bool          USE_HDR             =   true;
const uint32_t      NUM_BALLS           =   10;

// World
Camera* cam;
Scene scene;
	DirectionalLight *sunLight;
	Mesh* godRaySource;
	Mesh *ground;
	Mesh *model, *model2, *model3;
	std::vector<Mesh*> balls;

void setupScene();
void tick();

OBJLoader houseLoader;
bool asyncLoadComplete = false;
void asyncLoadTask() {
	bool loaded = houseLoader.load("models/small_colonial_house/houseSF.obj", "models/small_colonial_house/");
	CHECK(loaded);
	asyncLoadComplete = true;
}

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = WINDOW_TITLE;
	conf.rendererType = ERendererType::Deferred;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	// camera
	float aspect_ratio = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOVY, aspect_ratio, CAMERA_Z_NEAR, CAMERA_Z_FAR));
	cam->move(CAMERA_POSITION);

	std::thread asyncLoadWorker(asyncLoadTask);

	setupScene();

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	asyncLoadWorker.join();

	return 0;
}

void setupScene() {
	sunLight = new DirectionalLight(SUN_DIRECTION, glm::vec3(1.0f, 1.0f, 1.0f));
	scene.add(sunLight);

	scene.add(new PointLight(glm::vec3(20.0f, 30.0f, 50.0f), 5.0f * glm::vec3(1.0f, 1.0f, 1.0f)));
	scene.add(new PointLight(glm::vec3(-50.0f, 30.0f, 50.0f), 15.0f * glm::vec3(1.0f, 1.0f, 1.0f)));

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
	GLuint cubeTexture = pathos::loadCubemapTexture(cubeImg);

	GLuint tex = pathos::loadTexture(loadImage("resources/154.jpg"), true, true);
	GLuint tex_norm = pathos::loadTexture(loadImage("resources/154_norm.jpg"), true, false);

	auto material_texture = new TextureMaterial(tex);
	auto material_color = new ColorMaterial;
	{
		auto color = static_cast<ColorMaterial*>(material_color);
		color->setAlbedo(4.0f, 0.0f, 0.0f);
		color->setAlpha(1.0f);
		color->setMetallic(0.5f);
		color->setRoughness(0.1f);
	}
	auto material_cubemap = new CubeEnvMapMaterial(cubeTexture);
	auto material_wireframe = new WireframeMaterial(1.0f, 0.0f, 1.0f, 1.0f);

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
	ground->getTransform().appendScale(100.0f);
	ground->getTransform().appendRotation(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ground->getTransform().appendMove(0.0f, -30.0f, 0.0f);
	ground->castsShadow = false;

	model = new Mesh(geom_plane, material_pbr);
	model->getTransform().appendScale(5.0f, 5.0f, 5.0f);
	model->getTransform().appendRotation(glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model->getTransform().appendMove(-120.0f, 0.0f, -30.0f);
	model->doubleSided = true;

	model2 = new Mesh(geom_sphere, material_color);
	model2->getTransform().appendScale(3.0f);
	model2->getTransform().appendRotation(glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model2->getTransform().appendMove(50.0f, 0.0f, 0.0f);

	model3 = new Mesh(geom_cube, material_wireframe);
	model3->getTransform().appendMove(35.0f, 0.0f, 0.0f);

	for (auto i = 0u; i < NUM_BALLS; ++i) {
		Mesh* ball = new Mesh(geom_sphere, material_pbr);
		ball->getTransform().appendScale(2.0f + (float)i * 0.1f);
		ball->getTransform().appendMove((float)i * 5.0f, 0.0f, -30.0f * i);
		balls.push_back(ball);
		scene.add(ball);
	}

	godRaySource = new Mesh(geom_sphere, material_color);
	godRaySource->getTransform().appendScale(10.0f);
	godRaySource->getTransform().appendMove(0.0f, 300.0f, -500.0f);

	// add to scene
	scene.add(ground);
	scene.add(model);
	scene.add(model2);
	scene.add(model3);
	scene.sky = new Skybox(cubeTexture);
	//scene.sky = new AtmosphereScattering;
	scene.godRaySource = godRaySource;
}

void tick()
{
	if(asyncLoadComplete) {
		asyncLoadComplete = false;
		ColorMaterial* house_material = new ColorMaterial;
		house_material->setAlbedo(1.0f, 1.0f, 1.0f);
		house_material->setMetallic(0.0f);
		house_material->setRoughness(0.0f);
		Mesh* house = houseLoader.craftMeshFromAllShapes();
		for (int32 i = 0; i < (int32)house->getMaterials().size(); ++i) {
			house->setMaterial(i, house_material);
		}
		house->getTransform().appendRotation(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		house->getTransform().appendMove(-5.0f, -3.0f, -5.0f);
		house->getTransform().appendScale(10.0f);
		scene.add(house);
		houseLoader.unload();
	}

	if (gConsole->isVisible() == false) {
		float speedX = 1.0f, speedY = 1.0f;
		float dx   = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		float dz   = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		float rotY = gEngine->isDown('q') ? -0.5f   : gEngine->isDown('e') ? 0.5f   : 0.0f;
		float rotX = gEngine->isDown('z') ? -0.5f   : gEngine->isDown('x') ? 0.5f   : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}
	
	//model->getTransform().appendMove(0, 20, 0);
	//model->getTransform().appendRotation(0.01f, glm::vec3(0.0f, 0.5f, 1.0f));
	//model->getTransform().appendMove(0, -20, 0);

	model2->getTransform().appendMove(-60, 0, 0);
	model2->getTransform().appendRotation(0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
	model2->getTransform().appendMove(60, 0, 0);

	for (auto& ball : balls) {
		ball->getTransform().prependRotation(0.005f, glm::vec3(0.0f, 1.0f, 1.0f));
	}

#if !(_DEBUG)
	char title[256];
	sprintf_s(title, "%s (Elapsed: %.2f ms)", WINDOW_TITLE, gEngine->getMilliseconds());
	glutSetWindowTitle(title); // #todo: why this terminates the application in debug build after switching glLoadGen to gl3w?
#endif

}
