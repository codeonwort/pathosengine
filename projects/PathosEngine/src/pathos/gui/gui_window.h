#pragma once

#include "badger/types/int_types.h"

#include <functional>
#include <string>
#include <map>

namespace pathos {

	struct GUIWindowCreateParams {
		// Process arguments
		int argc;
		char** argv;

		// Window shape
		int32 width;
		int32 height;
		bool fullscreen;
		const char* title;
		
		// OpenGL setup
		int32 glMajorVersion;
		int32 glMinorVersion;
		bool glDebugContext;

		// Callbacks
		std::function<void()> onIdle;
		std::function<void()> onDisplay;
		std::function<void(int w, int h)> onReshape;
		std::function<void(unsigned char ascii, int x, int y)> onKeyDown;
		std::function<void(unsigned char ascii, int x, int y)> onKeyUp;
	};

	class GUIWindow {

	public:
		static std::map<int, GUIWindow*> handleToWindow;
		
	public:
		GUIWindow();
		virtual ~GUIWindow();

		// Actually create a window and show it to screen
		void create(const GUIWindowCreateParams& createParams);
		void startMainLoop();
		void stopMainLoop();

		void onIdle();
		void onDisplay();
		void onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY);
		void onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY);
		void onReshape(int32 newWidth, int32 newHeight);

		void setTitle(const char* newTitle);
		void setTitle(std::string&& newTitle);
		//void hideToTaskbar();
		//void flashTaskbar();

	private:
		bool initialized;

		int32 windowWidth;
		int32 windowHeight;
		bool bFullscreen;
		std::string title;

		int nativeHandle;

		std::function<void()> callback_onIdle;
		std::function<void()> callback_onDisplay;
		std::function<void(int32, int32)> callback_onReshape;
		std::function<void(uint8, int32, int32)> callback_onKeyDown;
		std::function<void(uint8, int32, int32)> callback_onKeyUp;

	};

}
