#pragma once

/*
dir2 / foo2.h.
A blank line
C system files.
C++ system files.
A blank line
Other libraries' .h files.
Your project's .h files.
*/

#include <stdint.h>
#include <memory>
#include <string>
#include <list>

namespace pathos {

	class ConsoleWindow {

	public:
		ConsoleWindow();
		ConsoleWindow(const ConsoleWindow&) = delete;
		ConsoleWindow& operator=(const ConsoleWindow&) = delete;
		~ConsoleWindow();

		bool initialize(uint16_t width, uint16_t height);
		void render();
		void toggle();

		bool isVisible() const;

		void onKeyPress(unsigned char ascii);

		class Label* addLine(const wchar_t* text);

	private:
		bool initialized;
		bool visible;
		uint16_t windowWidth;
		uint16_t windowHeight;

		class OverlayRenderer* renderer;
		class DisplayObject2D* root;
		class Rectangle* background;
		class Label* inputText;
		std::list<class Label*> textList;

		std::wstring input;

	};

}