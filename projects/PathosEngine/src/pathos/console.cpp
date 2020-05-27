#include "console.h"
#include "engine.h"
#include "pathos/render/render_overlay.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/brush.h"
#include "pathos/util/string_conversion.h"
#include "pathos/util/math_lib.h"

namespace pathos {

	static constexpr float LEFT_MARGIN = 10.0f;
	static constexpr float LINE_GAP = 20.0f;
	static constexpr size_t MAX_LINES = 18;
	static constexpr size_t MAX_HISTORY = 64;

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

	bool ConsoleWindow::initialize(uint16 width, uint16 height) {
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

	void ConsoleWindow::renderConsoleWindow(RenderCommandList& cmdList) {
		if (visible) {
			renderer->renderOverlay(cmdList, root);
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
			if (currentInput.size() > 0) {
				currentInput = currentInput.substr(0, currentInput.size() - 1);
			}
		} else if (ascii == 13) {
			// enter
			addLine(currentInput.data(), true);
			currentInput = L"";
		} else {
			currentInput += ascii;
		}

		updateInputLine();
	}

	void ConsoleWindow::showPreviousHistory() {
		if (inputHistory.size() > 0) {
			inputHistoryCursor = pathos::max(0, inputHistoryCursor - 1);
			currentInput = inputHistory[inputHistoryCursor];
			updateInputLine();
		}
	}

	void ConsoleWindow::showNextHistory() {
		if (inputHistory.size() > 0) {
			inputHistoryCursor = pathos::min((int32)inputHistory.size(), inputHistoryCursor + 1);
			currentInput = (inputHistoryCursor == (int32)inputHistory.size()) ? L"" : inputHistory[inputHistoryCursor];
			updateInputLine();
		}
	}

	Label* ConsoleWindow::addLine(const char* text, bool addToHistory) {
		std::wstring buffer;
		pathos::MBCS_TO_WCHAR(text, buffer);

		return addLine(buffer.data(), addToHistory);
	}

	Label* ConsoleWindow::addLine(const wchar_t* text, bool addToHistory) {
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
		if (addToHistory) {
			addInputHistory(text);
		}

		return label;
	}

	void ConsoleWindow::updateInputLine() {
		std::wstring input2 = L"> " + currentInput;
		inputText->setVisible(true);
		inputText->setText(input2.data());
	}

	void ConsoleWindow::evaluate(const wchar_t* text) {
		std::string command;
		pathos::WCHAR_TO_MBCS(text, command);

		auto ix = command.find(' ');
		std::string header = ix == string::npos ? command : command.substr(0, ix);

		// Execute registered procedure if exists
		if (gEngine->execute(command)) {
			return;
		}

		// Is it a cvar?
		if (auto cvar = ConsoleVariableManager::find(header.data())) {
			std::string msg = command.substr(ix + 1);
			if (ix == string::npos) {
				cvar->print(this);
			} else {
				cvar->parse(msg.data(), this);
			}
		}
	}

	void ConsoleWindow::addInputHistory(const wchar_t* inputText) {
		inputHistory.push_back(inputText);
		if (inputHistory.size() > MAX_HISTORY) {
			inputHistory.erase(inputHistory.begin());
		}
		inputHistoryCursor = (int32)inputHistory.size();
	}

}

namespace pathos {

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
