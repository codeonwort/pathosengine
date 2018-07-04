#include "console.h"
#include "pathos/render/render_overlay.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/brush.h"

namespace pathos {

	static constexpr float LEFT_MARGIN = 10.0f;
	static constexpr float LINE_GAP = 20.0f;
	static constexpr size_t MAX_LINES = 18;

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

		inputText = new Label(L"> ");
		inputText->setX(10.0f);
		inputText->setY(height - 30.0f);
		root->addChild(inputText);

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

	bool ConsoleWindow::isVisible() const {
		return visible;
	}

	void ConsoleWindow::onKeyPress(unsigned char ascii) {
		if (ascii == 0x08) {
			// backspace
			if (input.size() > 0) {
				input = input.substr(0, input.size() - 1);
			}
		} else if (ascii == 13) {
			// enter
			addLine(input.data());
			input = L"";
		} else {
			input += ascii;
		}

		if (input.size() == 0) {
			inputText->setVisible(false);
		} else {
			inputText->setVisible(true);
			inputText->setText(input.data());
		}
	}

	Label* ConsoleWindow::addLine(const char* text) {
		int size = MultiByteToWideChar(CP_ACP, 0, text, -1, NULL, NULL);
		std::vector<wchar_t> buffer(size, 0);
		MultiByteToWideChar(CP_ACP, 0, text, strlen(text)+1, buffer.data(), size);
		return addLine(buffer.data());
	}

	Label* ConsoleWindow::addLine(const wchar_t* text) {
		if (wcslen(text) == 0) {
			return nullptr;
		}

		Label* label = new Label(text);
		label->setX(LEFT_MARGIN);
		label->setY(textList.size() * LINE_GAP);

		textList.push_back(label);
		root->addChild(label);

		if (textList.size() > MAX_LINES) {
			Label* old = textList.front();
			root->removeChild(old);
			textList.pop_front();
			delete old;
			for (Label* label : textList) {
				label->setY(label->getY() - LINE_GAP);
			}
		}

		evaluate(text);

		return label;
	}

	void ConsoleWindow::evaluate(const wchar_t* text) {
		int size = WideCharToMultiByte(CP_ACP, 0, text, -1, NULL, 0, NULL, NULL);
		std::vector<char> buffer(size, 0);
		WideCharToMultiByte(CP_ACP, 0, text, -1, buffer.data(), size, NULL, NULL);
		std::string command(buffer.data());

		// is it just a cvar name?
		if (auto cvar = ConsoleVariableManager::find(command.data())) {
			cvar->print(this);
		}
	}

	///////////////////////////////////////////////////////////
	// ConsoleVariable

	std::vector<ConsoleVariableBase*> ConsoleVariableManager::registry;

	ConsoleVariableBase* ConsoleVariableManager::find(const char* name) {
		for (auto it = registry.begin(); it != registry.end(); ++it) {
			ConsoleVariableBase* cvar = *it;
			if(_strcmpi(cvar->name.data(), name) == 0) {
				return cvar;
			}
		}
		return nullptr;
	}

	ConsoleVariableBase::ConsoleVariableBase() {
		ConsoleVariableManager::registry.push_back(this);
	}

}
