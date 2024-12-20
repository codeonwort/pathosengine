#pragma once

#include "badger/types/int_types.h"

#include "pathos/input/input_constants.h"

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
		std::function<void()> onClose;
		std::function<void()> onIdle;
		std::function<void()> onDisplay;
		std::function<void(int w, int h)> onReshape;
		std::function<void(unsigned char ascii, int x, int y)> onKeyDown;
		std::function<void(unsigned char ascii, int x, int y)> onKeyUp;
		std::function<void(InputConstants)> onSpecialKeyDown;
		std::function<void(InputConstants)> onSpecialKeyUp;
		std::function<void(InputConstants, int32 mouseX, int32 mouseY)> onMouseDown;
		std::function<void(InputConstants, int32 mouseX, int32 mouseY)> onMouseUp;
		std::function<void(int32 mouseX, int32 mouseY)> onMouseDrag;
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

		void updateWindow_renderThread();

		void onClose();
		void onIdle();
		void onDisplay();
		void onKeyDown(uint8 ascii, int32 mouseX, int32 mouseY);
		void onKeyUp(uint8 ascii, int32 mouseX, int32 mouseY);
		void onReshape(int32 newWidth, int32 newHeight);
		void checkSpecialKeyDown(int specialKey);    // modifiers(ctrl/shift/alt), arrows
		void checkSpecialKeyUp(int specialKey);      // modifiers(ctrl/shift/alt), arrows
		void onMouseFunc(int button, int state, int x, int y);
		void onMouseDrag(int x, int y);

		void getSize(uint32* outWidth, uint32* outHeight) const;

		void setTitle(const char* newTitle);
		void setTitle(std::string&& newTitle);
		void setSize(uint32 newWidth, uint32 newHeight);
		//void hideToTaskbar();
		//void flashTaskbar();

		void setFullscreen(bool enable);

	private:
		bool initialized;

		int32 windowWidth;
		int32 windowHeight;
		bool bFullscreen;
		std::string title;

		int nativeHandle;

		std::function<void()> callback_onClose;
		std::function<void()> callback_onIdle;
		std::function<void()> callback_onDisplay;
		std::function<void(int32, int32)> callback_onReshape;
		std::function<void(uint8, int32, int32)> callback_onKeyDown;
		std::function<void(uint8, int32, int32)> callback_onKeyUp;
		std::function<void(InputConstants modifier)> callback_onSpecialKeyDown;
		std::function<void(InputConstants modifier)> callback_onSpecialKeyUp;
		std::function<void(InputConstants mouseInput, int32 mouseX, int32 mouseY)> callback_onMouseDown;
		std::function<void(InputConstants mouseInput, int32 mouseX, int32 mouseY)> callback_onMouseUp;
		std::function<void(int32 mouseX, int32 mouseY)> callback_onMouseDrag;

	};

}
