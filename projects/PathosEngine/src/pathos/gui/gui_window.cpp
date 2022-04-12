#include "gui_window.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

#include "pathos/util/log.h"
#include "pathos/util/gl_context_manager.h"

#include "GL/freeglut.h"
#include <stdio.h>
#include <array>

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

	static void onGlutClose() {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->onClose();
	}

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

	static void onGlutSpecialKeyDown(int specialKey, int mouseX, int mouseY) {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->checkSpecialKeyDown(specialKey);
	}

	static void onGlutSpecialKeyUp(int specialKey, int mouseX, int mouseY) {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->checkSpecialKeyUp(specialKey);
	}

	static void onGlutMouseFunc(int button, int state, int x, int y) {
		GUIWindow* window = GUIWindow::handleToWindow[glutGetWindow()];
		window->onMouseFunc(button, state, x, y);
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

		int argc                   = createParams.argc;
		char** argv                = createParams.argv;

		windowWidth                = createParams.width;
		windowHeight               = createParams.height;
		bFullscreen                = createParams.fullscreen;
		title                      = createParams.title;

		callback_onClose           = createParams.onClose;
		callback_onIdle            = createParams.onIdle;
		callback_onDisplay         = createParams.onDisplay;
		callback_onKeyDown         = createParams.onKeyDown;
		callback_onKeyUp           = createParams.onKeyUp;
		callback_onSpecialKeyDown  = createParams.onSpecialKeyDown;
		callback_onSpecialKeyUp    = createParams.onSpecialKeyUp;
		callback_onReshape         = createParams.onReshape;
		callback_onMouseDown       = createParams.onMouseDown;
		callback_onMouseUp         = createParams.onMouseUp;

		CHECKF(windowWidth > 0 && windowHeight > 0, "Invalid window size");
		CHECKF(title.size() > 0, "Invalid window title");

		// Silently fix window size
		windowWidth = badger::max<int32>(400, windowWidth);
		windowHeight = badger::max<int32>(300, windowHeight);
		if (title.empty()) title = "Title here";

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

		// X*10000 + Y*100 + Z where X = major, Y = minor, and Z = patch.
		const int32 glutVersion = glutGet(GLUT_VERSION);
		int32 glutMajorVersion = glutVersion / 10000;
		int32 glutMinorVersion = (glutVersion % 10000) / 100;
		int32 glutPatchVersion = glutVersion % 100;
		LOG(LogInfo, "[ThirdParty] GUI Backend: Freeglut %d.%d.%d", glutMajorVersion, glutMinorVersion, glutPatchVersion);
		
		nativeHandle = glutCreateWindow(title.c_str());
		OpenGLContextManager::initialize();

		if (bFullscreen) {
			glutFullScreen();
		}

		glutCloseFunc(onGlutClose);
		glutIdleFunc(onGlutIdle);
		glutDisplayFunc(onGlutDisplay);
		glutReshapeFunc(onGlutReshape);
		glutKeyboardFunc(onGlutKeyDown);
		glutKeyboardUpFunc(onGlutKeyUp);
		glutSpecialFunc(onGlutSpecialKeyDown);
		glutSpecialUpFunc(onGlutSpecialKeyUp);
		glutMouseFunc(onGlutMouseFunc);

		GUIWindow::handleToWindow[nativeHandle] = this;

		OpenGLContextManager::returnContext();
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

	void GUIWindow::onClose()
	{
		callback_onClose();
	}

	void GUIWindow::onIdle()
	{
		callback_onIdle();
	}

	void GUIWindow::onDisplay()
	{
		callback_onDisplay();

		OpenGLContextManager::takeContext();
		glutSwapBuffers();
		glutPostRedisplay();
		OpenGLContextManager::returnContext();
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

	void GUIWindow::setTitle(const char* newTitle)
	{
		title = newTitle;

		glutSetWindow(nativeHandle);
		glutSetWindowTitle(title.c_str());
	}

	void GUIWindow::setTitle(std::string&& newTitle)
	{
		title = std::move(newTitle);

		glutSetWindow(nativeHandle);
		glutSetWindowTitle(title.c_str());
	}

	void GUIWindow::checkSpecialKeyDown(int specialKey)
	{
		// Modifiers
		int modifiers = glutGetModifiers();
		if (modifiers & GLUT_ACTIVE_SHIFT) {
			callback_onSpecialKeyDown(InputConstants::SHIFT);
		}
		if (modifiers & GLUT_ACTIVE_CTRL) {
			callback_onSpecialKeyDown(InputConstants::CTRL);
		}
		if (modifiers & GLUT_ACTIVE_ALT) {
			callback_onSpecialKeyDown(InputConstants::ALT);
		}

		// Arrows
		if (specialKey == GLUT_KEY_LEFT) {
			callback_onSpecialKeyDown(InputConstants::KEYBOARD_ARROW_LEFT);
		} else if (specialKey == GLUT_KEY_RIGHT) {
			callback_onSpecialKeyDown(InputConstants::KEYBOARD_ARROW_RIGHT);
		} else if (specialKey == GLUT_KEY_UP) {
			callback_onSpecialKeyDown(InputConstants::KEYBOARD_ARROW_UP);
		} else if (specialKey == GLUT_KEY_DOWN) {
			callback_onSpecialKeyDown(InputConstants::KEYBOARD_ARROW_DOWN);
		}
	}

	void GUIWindow::checkSpecialKeyUp(int specialKey)
	{
		// Modifiers
		int modifiers = glutGetModifiers();
		if (0 == (modifiers & GLUT_ACTIVE_SHIFT)) {
			callback_onSpecialKeyUp(InputConstants::SHIFT);
		}
		if (0 == (modifiers & GLUT_ACTIVE_CTRL)) {
			callback_onSpecialKeyUp(InputConstants::CTRL);
		}
		if (0 == (modifiers & GLUT_ACTIVE_ALT)) {
			callback_onSpecialKeyUp(InputConstants::ALT);
		}

		// Arrows
		if (specialKey == GLUT_KEY_LEFT) {
			callback_onSpecialKeyUp(InputConstants::KEYBOARD_ARROW_LEFT);
		} else if (specialKey == GLUT_KEY_RIGHT) {
			callback_onSpecialKeyUp(InputConstants::KEYBOARD_ARROW_RIGHT);
		} else if (specialKey == GLUT_KEY_UP) {
			callback_onSpecialKeyUp(InputConstants::KEYBOARD_ARROW_UP);
		} else if (specialKey == GLUT_KEY_DOWN) {
			callback_onSpecialKeyUp(InputConstants::KEYBOARD_ARROW_DOWN);
		}
	}

	void GUIWindow::onMouseFunc(int button, int state, int x, int y) {
		InputConstants input = (button == GLUT_LEFT_BUTTON) ? InputConstants::MOUSE_LEFT_BUTTON
			: (button == GLUT_MIDDLE_BUTTON) ? InputConstants::MOUSE_MIDDLE_BUTTON
			: (button == GLUT_RIGHT_BUTTON) ? InputConstants::MOUSE_RIGHT_BUTTON
			: InputConstants::UNDEFINED;

		if (button == 3 || button == 4) {
			// #todo: These are mouse wheel input, but it seems there are no constants for them.
		} else {
			CHECKF(input != InputConstants::UNDEFINED, "Unexpected mouse button input");
		}

		if (state == GLUT_DOWN) {
			callback_onMouseDown(input, (int32)x, (int32)y);
		} else if (state == GLUT_UP) {
			callback_onMouseUp(input, (int32)x, (int32)y);
		} else {
			CHECKF(0, "Unexpected mouse button state");
		}
	}

}
