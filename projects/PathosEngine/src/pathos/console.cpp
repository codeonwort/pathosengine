#include "console.h"
#include "engine.h"
#include "pathos/render/render_overlay.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/brush.h"
#include "pathos/util/string_conversion.h"
#include "pathos/util/math_lib.h"

#define MAX_HINT_LIST       8
#define HINT_LABEL_WIDTH    300
#define HINT_LABEL_HEIGHT   20

namespace pathos {

	static constexpr float LEFT_MARGIN = 10.0f;
	static constexpr float LINE_GAP = 20.0f;
	static constexpr size_t MAX_LINES = 18;
	static constexpr size_t MAX_HISTORY = 64;

	ConsoleWindow::ConsoleWindow(OverlayRenderer* renderer2D)
		: initialized(false)
		, visible(true)
		, windowWidth(0)
		, windowHeight(0)
		, renderer(renderer2D)
		, root(nullptr)
		, background(nullptr)
		, inputText(nullptr)
		, hintBackground(nullptr)
		, inputHistoryCursor(0)
	{
		CHECK(renderer != nullptr);
	}

	ConsoleWindow::~ConsoleWindow() {
		if (root) delete root;
		if (background) delete background;
	}

	bool ConsoleWindow::initialize(uint16 width, uint16 height) {
		windowWidth = width;
		windowHeight = height;

		root = DisplayObject2D::createRoot();

		background = new Rectangle(windowWidth, windowHeight);
		background->setBrush(new SolidColorBrush(0.0f, 0.0f, 0.1f));
		root->addChild(background);

		inputText = new Label(L"> ");
		inputText->setX(10.0f);
		inputText->setY(height - 30.0f);
		root->addChild(inputText);

		hintBackground = new Rectangle(HINT_LABEL_WIDTH, 10.0f + (MAX_HINT_LIST + 1) * HINT_LABEL_HEIGHT);
		hintBackground->setBrush(new SolidColorBrush(0.0f, 0.1f, 0.1f));
		hintBackground->setX(10.0f);
		hintBackground->setY(windowHeight - 40.0f - (MAX_HINT_LIST + 1) * HINT_LABEL_HEIGHT);
		hintBackground->setVisible(false);
		root->addChild(hintBackground);
		for (int32 i = 0; i <= MAX_HINT_LIST; ++i) {
			Label* hint = new Label(L"...");
			hint->setX(0.0f);
			hint->setY((float)(i * HINT_LABEL_HEIGHT));
			hint->setVisible(false);
			hintList.push_back(hint);
			root->addChild(hint);
		}

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

		updateHint(currentInput, currentInput.size() == 0);
	}

	void ConsoleWindow::updateHint(const std::wstring& currentText, bool forceHide) {
		if (forceHide) {
			hintBackground->setVisible(false);
			for (int32 i = 0; i <= MAX_HINT_LIST; ++i) {
				hintList[i]->setVisible(false);
			}
			return;
		}

		enum class HintCategory {
			Exec = 0,
			CVar = 1
		};

		std::vector<std::wstring> allCommands;
		std::vector<HintCategory> hintCategories;

		for (const auto& it : gEngine->getExecMap()) {
			std::wstring wcmd;
			pathos::MBCS_TO_WCHAR(it.first, wcmd);
			allCommands.push_back(wcmd);
			hintCategories.push_back(HintCategory::Exec);
		}
		std::string currentTextMBCS;
		std::vector<std::string> matchingCVars;
		pathos::WCHAR_TO_MBCS(currentText, currentTextMBCS);
		ConsoleVariableManager::get().getCVarNamesWithPrefix(currentTextMBCS.c_str(), matchingCVars);
		for (const auto& it : matchingCVars) {
			std::wstring cvarNameW;
			pathos::MBCS_TO_WCHAR(it, cvarNameW);
			allCommands.push_back(cvarNameW);
			hintCategories.push_back(HintCategory::CVar);
		}

		int32 hintIx = 0;
		int32 totalMatching = 0;
		for (int32 i = 0; i < allCommands.size(); ++i) {
			const std::wstring& cmd = allCommands[i];
			const HintCategory cat = hintCategories[i];
			if (cmd.find(currentInput) == 0) {
				totalMatching += 1;
				if (hintIx > MAX_HINT_LIST) {
					continue;
				} else if (hintIx == MAX_HINT_LIST) {
					hintList[MAX_HINT_LIST]->setText(L"...");
					hintList[MAX_HINT_LIST]->setColor(vector3(1.0f, 1.0f, 1.0f));
				} else {
					hintList[hintIx]->setText(cmd.c_str());
					vector3 c = (cat == HintCategory::Exec) ? vector3(1.0f, 1.0f, 0.0f) : vector3(1.0f, 1.0f, 1.0f);
					hintList[hintIx]->setColor(c);
				}
				hintIx += 1;
			}
		}
		if (totalMatching > MAX_HINT_LIST) {
			wchar_t buffer[256];
			swprintf_s(buffer, L"%d more...", totalMatching - MAX_HINT_LIST);
			hintList[MAX_HINT_LIST]->setText(buffer);
		}

		float baseY = windowHeight - 40.0f - (HINT_LABEL_HEIGHT * hintIx);
		for (int32 i = 0; i < hintIx; ++i) {
			hintList[i]->setVisible(true);
			hintList[i]->setX(20.0f);
			hintList[i]->setY(baseY + i * HINT_LABEL_HEIGHT);
		}
		for (int32 i = hintIx; i <= MAX_HINT_LIST; ++i) {
			hintList[i]->setVisible(false);
		}
		hintBackground->setVisible(hintIx != 0);
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
		if (auto cvar = ConsoleVariableManager::get().find(header.data())) {
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

	pathos::ConsoleVariableManager& ConsoleVariableManager::get()
	{
		static ConsoleVariableManager inst;
		return inst;
	}

	ConsoleVariableBase* ConsoleVariableManager::find(const char* name) {
		for (auto it = registry.begin(); it != registry.end(); ++it) {
			ConsoleVariableBase* cvar = *it;
			if(_strcmpi(cvar->name.data(), name) == 0) {
				return cvar;
			}
		}
		return nullptr;
	}

	void ConsoleVariableManager::registerCVar(ConsoleVariableBase* cvar)
	{
		std::lock_guard<std::mutex> lockRegistry(registryLock);
		registry.push_back(cvar);
	}

	void ConsoleVariableManager::getCVarNamesWithPrefix(const char* prefix, std::vector<std::string>& outNames)
	{
		outNames.clear();
		for (const auto& it : registry) {
			if (it->name.find(prefix) == 0) {
				outNames.push_back(it->name);
			}
		}
	}

	ConsoleVariableBase::ConsoleVariableBase() {
		ConsoleVariableManager::get().registerCVar(this);
	}

}
