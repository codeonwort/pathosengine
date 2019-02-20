#include "glm/gtx/transform.hpp"
#include "pathos/core_minimal.h"
#include "pathos/console.h"
#include "pathos/render/render_forward.h"
#include "pathos/render/envmap.h"
#include "pathos/mesh/mesh.h"
#include "pathos/camera/camera.h"
#include "pathos/light/light.h"
#include "pathos/loader/imageloader.h"
#include "pathos/text/textmesh.h"
#include "pathos/text/font_mgr.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/resource_finder.h"

// Overlay (2D rendering)
#include "pathos/render/render_overlay.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/label.h"

using namespace pathos;

#define DEBUG_FONT_TEXTURE 0

// Configurations
constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;
constexpr float FOV = 90.0f;
const glm::vec3 CAMERA_POSITION(0.0f, 0.0f, 10.0f);
constexpr char* TITLE = "Test: Text Rendering";

// Camera, scene and renderer
Camera* cam;
Scene scene;
MeshForwardRenderer* renderer;
OverlayRenderer* overlayRenderer;

// 3D objects
TextMesh *label;
Skybox* sky;
#if DEBUG_FONT_TEXTURE
Mesh* debug_overlayLabel;
#endif

// 2D objects (overlay)
DisplayObject2D* overlayRoot;
pathos::Rectangle* rect0;
Label* label0;

// Lights
PointLight *plight;
DirectionalLight *dlight;

void setupOverlay();
void setupModel();
void setupSkybox();

void tick();
void render();

int main(int argc, char** argv) {
	// engine configuration
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = TITLE;
	conf.tick         = tick;
	conf.render       = render;
	Engine::init(&argc, argv, conf);

	// camera
	float ar = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, ar, 1.0f, 1000.f));
	cam->move(CAMERA_POSITION);

	// renderer
	renderer = new MeshForwardRenderer;
	overlayRenderer = new OverlayRenderer;

	// 3d objects
	setupOverlay();
	setupModel();
	setupSkybox();

	// start the main loop
	gEngine->start();

	return 0;
}

void setupModel() {
	plight = new PointLight(glm::vec3(5.0f, 30.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	dlight = new DirectionalLight(glm::vec3(0.1f, -1.0f, 2.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	//FontManager::loadAdditionalGlyphs("hangul", L'ㄱ', L'힣');

	label = new TextMesh("hangul");
	label->setText(L"ㅁㄴㅇㄹ / 오픈지엘 English Text\n가나다라마바사", 0xff0000);
	label->getTransform().appendScale(20.0f);

#if DEBUG_FONT_TEXTURE
	debug_overlayLabel = new Mesh(
		new PlaneGeometry(20.0f, 20.0f),
		new AlphaOnlyTextureMaterial(Label::getFontTexture()));
#endif

	scene.add(plight);
	scene.add(dlight);
	scene.add(label);
#if DEBUG_FONT_TEXTURE
	scene.add(debug_overlayLabel);
#endif

	renderer->getShadowMap()->setProjection(
		glm::ortho(-200.0f, 200.0f, -200.0f, 100.0f, -200.0f, 500.0f));
}

void setupSkybox() {
	const char* cubeImgName[6] = {
		"cubemap1/pos_x.jpg", "cubemap1/neg_x.jpg",
		"cubemap1/pos_y.jpg", "cubemap1/neg_y.jpg",
		"cubemap1/pos_z.jpg", "cubemap1/neg_z.jpg"
	};
	FIBITMAP* cubeImg[6];
	for (int i = 0; i < 6; i++) cubeImg[i] = loadImage(cubeImgName[i]);
	GLuint cubeTex = loadCubemapTexture(cubeImg);
	sky = new Skybox(cubeTex);

	scene.skybox = sky;
}

void setupOverlay() {
	overlayRoot = DisplayObject2D::createRoot();
	rect0 = new pathos::Rectangle(400.0f, 500.0f);
	overlayRoot->addChild(rect0);
}

void tick() {
	float speedX = 0.05f, speedY = 0.05f;
	float dx = 0.0f;
	float dz = 0.0f;
	float rotY = 0.0f;
	float rotX = 0.0f;
	if (gConsole->isVisible() == false) {
		dx = gEngine->isDown('a') ? -speedX : gEngine->isDown('d') ? speedX : 0.0f;
		dz = gEngine->isDown('w') ? -speedY : gEngine->isDown('s') ? speedY : 0.0f;
		rotY = gEngine->isDown('q') ? -0.5f : gEngine->isDown('e') ? 0.5f : 0.0f;
		rotX = gEngine->isDown('z') ? -0.5f : gEngine->isDown('x') ? 0.5f : 0.0f;
		cam->move(glm::vec3(dx, 0, dz));
		cam->rotateY(rotY);
		cam->rotateX(rotX);
	}

	static std::wstring txt = L"가나다라마바사";
	static bool first = true;
	if (first) {
		//for (wchar_t x = L'ㄱ'; x <= (L'ㄱ' + 100); ++x) txt += x;
		first = false;
	}

	static int cnt = 0;
	if (cnt++ > 10) {
		txt.push_back(txt.front());
		txt = txt.substr(1);
		cnt = 0;
	}
	//label->setText(txt, 0xff0000);
}

void render() {
	renderer->render(&scene, cam);
	//overlayRenderer->render(overlayRoot);
}
