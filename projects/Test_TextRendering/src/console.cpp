#include "console.h"
#include "pathos/render/render_overlay.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/brush.h"

namespace pathos {

	ConsoleWindow::ConsoleWindow() {
		initialized = false;
		visible = true;
		renderer = nullptr;
		root = nullptr;
		background = nullptr;
	}

	ConsoleWindow::~ConsoleWindow() {
		if (renderer) delete renderer;
		if (root) delete root;
		if (background) delete background;
	}

	bool ConsoleWindow::initialize(uint16_t width, uint16_t height) {
		windowWidth = width;
		windowHeight = height;
		
		renderer = new OverlayRenderer;
		root = DisplayObject2D::createRoot();
		background = new Rectangle(windowWidth, windowHeight);

		background->setBrush(new SolidColorBrush(0.0f, 0.0f, 0.1f));

		root->addChild(background);

		initialized = true;
		return initialized;
	}

	void ConsoleWindow::render() {
		if (visible) {
			renderer->render(root);
		}
	}

	void ConsoleWindow::toggle() {
		visible = !visible;
	}

	Label* ConsoleWindow::addLine(const wchar_t* text) {
		Label* label = new Label(text);
		label->setX(20.0f);
		label->setY(50.0f);

		textList.push_back(label);
		root->addChild(label);

		return label;
	}

}
