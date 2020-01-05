#include "gui_window.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include "GL/freeglut.h"
#include <stdio.h>
#include <algorithm>

#pragma comment(lib, "freeglut.lib")

static void onGlutError(const char* fmt, va_list ap) {
	fprintf(stderr, "onGlutError:");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");

	/* deInitialize the freeglut state */
	fprintf(stderr, "onGlutError: Calling glutExit()\n");
	glutExit();

	exit(1);
}

static void onGlutWarning(const char* fmt, va_list ap) {
	fprintf(stderr, "onGlutWarning:");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");

	/* deInitialize the freeglut state */
	fprintf(stderr, "onGlutWarning: Calling glutExit()\n");
	glutExit();

	exit(1);
}

namespace pathos {

	static void onGlutIdle() {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->onIdle();
	}

	static void onGlutDisplay() {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->onDisplay();
	}

	static void onGlutKeyDown(unsigned char ascii, int mouseX, int mouseY) {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->onKeyDown(ascii, (int32)mouseX, (int32)mouseY);
	}

	static void onGlutKeyUp(unsigned char ascii, int mouseX, int mouseY) {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->onKeyUp(ascii, (int32)mouseX, (int32)mouseY);
	}

	static void onGlutReshape(int width, int height) {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->onReshape((int32)width, (int32)height);
	}

	////////////////////////////////////////////////////////////////////////////

	std::map<int, GUIWindow*> GUIWindow::handleToWindow;

	GUIWindow::GUIWindow()
		: initialized(false)
		, windowWidth(0)
		, windowHeight(0)
		, bFullscreen(false)
		, title("title here")
		, nativeHandle(-1)
	{
		//
	}

	GUIWindow::~GUIWindow()
	{
	}

	void GUIWindow::create(const GUIWindowCreateParams& createParams)
	{
		if (initialized) {
			LOG(LogError, "This window is already created");
			return;
		}
		initialized = true;

		int argc           = createParams.argc;
		char** argv        = createParams.argv;

		windowWidth  = createParams.width;
		windowHeight = createParams.height;
		bFullscreen  = createParams.fullscreen;
		title        = createParams.title;

		callback_onIdle    = createParams.onIdle;
		callback_onDisplay = createParams.onDisplay;
		callback_onKeyDown = createParams.onKeyDown;
		callback_onKeyUp   = createParams.onKeyUp;
		callback_onReshape = createParams.onReshape;

		CHECK(windowWidth > 0 && windowHeight > 0);
		CHECK(title != nullptr);

		// Silently fix values
		windowWidth = std::max(400, windowWidth);
		windowHeight = std::max(300, windowWidth);
		if (title == nullptr) title = "title here";

		glutInitErrorFunc(onGlutError);
		glutInitWarningFunc(onGlutWarning);
		glutInit(&argc, argv);
		glutInitContextVersion(createParams.glMajorVersion, createParams.glMinorVersion);
		glutInitContextProfile(GLUT_CORE_PROFILE);
		if (createParams.glDebugContext) {
			glutInitContextFlags(GLUT_DEBUG);
		}
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
		glutInitWindowSize(windowWidth, windowHeight);

		nativeHandle = glutCreateWindow(title);

		glutIdleFunc(onGlutIdle);
		glutDisplayFunc(onGlutDisplay);
		glutReshapeFunc(onGlutReshape);
		glutKeyboardFunc(onGlutKeyDown);
		glutKeyboardUpFunc(onGlutKeyUp);

		GUIWindow::handleToWindow[nativeHandle] = this;
	}

	void GUIWindow::startMainLoop()
	{
		CHECK(initialized);

		LOG(LogInfo, "Start the main loop");
		glutMainLoop();
	}

	void GUIWindow::stopMainLoop()
	{
		CHECK(initialized);

		GUIWindow::handleToWindow[nativeHandle] = nullptr;

		glutLeaveMainLoop();
		LOG(LogInfo, "Stop the main loop");
	}

	void GUIWindow::onIdle()
	{
		callback_onIdle();
	}

	void GUIWindow::onDisplay()
	{
		callback_onDisplay();

		glutSwapBuffers();
		glutPostRedisplay();
	}

	void GUIWindow::onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY)
	{
		callback_onKeyDown(ascii, mouseX, mouseY);
	}

	void GUIWindow::onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY)
	{
		callback_onKeyUp(ascii, mouseX, mouseY);
	}

	void GUIWindow::onReshape(int32 newWidth, int32 newHeight)
	{
		callback_onReshape(newWidth, newHeight);
	}

}
