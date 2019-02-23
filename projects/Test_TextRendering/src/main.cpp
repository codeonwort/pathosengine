#include "pathos/core_minimal.h"
#include "pathos/render_minimal.h"
#include "pathos/text/textmesh.h"
#include "pathos/text/font_mgr.h"
using namespace pathos;

// Configurations
constexpr int WINDOW_WIDTH      = 1920;
constexpr int WINDOW_HEIGHT     = 1080;
constexpr char* TITLE           = "Test: Text Rendering";
constexpr float FOV             = 90.0f;
const glm::vec3 CAMERA_POSITION = glm::vec3(0.0f, 0.0f, 10.0f);

Camera* cam;
Scene scene;

// 3D objects
TextMesh *label;
Skybox* sky;

// Lights
PointLight *plight;

void setupModel();
void setupSkybox();

void tick();

int main(int argc, char** argv) {
	EngineConfig conf;
	conf.windowWidth  = WINDOW_WIDTH;
	conf.windowHeight = WINDOW_HEIGHT;
	conf.title        = TITLE;
	conf.rendererType = ERendererType::Forward;
	conf.tick         = tick;
	Engine::init(&argc, argv, conf);

	float ar = static_cast<float>(conf.windowWidth) / static_cast<float>(conf.windowHeight);
	cam = new Camera(new PerspectiveLens(FOV / 2.0f, ar, 1.0f, 1000.f));
	cam->move(CAMERA_POSITION);

	setupModel();
	setupSkybox();

	gEngine->setWorld(&scene, cam);
	gEngine->start();

	return 0;
}

void setupModel() {
	plight = new PointLight(glm::vec3(5.0f, 30.0f, 5.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	//FontManager::loadAdditionalGlyphs("hangul", L'ㄱ', L'힣');

	label = new TextMesh("hangul");
	label->setText(L"ㅁㄴㅇㄹ / 오픈지엘 English Text\n가나다라마바사", 0xff0000);
	label->getTransform().appendScale(20.0f);

	scene.add(plight);
	scene.add(label);
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

	scene.sky = sky;
}

void tick() {
	const float speedX = 0.5f;
	const float speedY = 0.5f;
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
