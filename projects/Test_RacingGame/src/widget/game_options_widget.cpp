#include "game_options_widget.h"
#include "world/world_racing_title.h"

#include "pathos/overlay/label.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/input/input_manager.h"
#include "pathos/gui/gui_window.h"
#include "pathos/util/log.h"

// #todo: Query available display resolutions from OS.
static const int32 RESOLUTION_PRESETS[][2] = {
	{ 1024, 768 },
	{ 1600, 900 },
	{ 1920, 1080 },
	{ 2560, 1440 },
	{ 3840, 2160 },
};
static const int32       NUM_RESOLUTIONS   = _countof(RESOLUTION_PRESETS);
static const std::string LABEL_FONT        = "defaultLarge";
static const float       LABEL_SPACE_Y     = 50.0f;
static const vector3     LABEL_COLOR       = vector3(0.4f, 0.4f, 0.4f);
static const vector3     LABEL_COLOR_FOCUS = vector3(1.0f, 1.0, 1.0);

// ------------------------------------------------------------------
// ChoiceControl

ChoiceControl::ChoiceControl(const std::wstring& label, const std::vector<std::wstring>& choices) {
	contents = choices;
	selectedContent = (int32)(choices.size()) - 1;

	header = new pathos::Label(label.c_str());
	header->setFont(LABEL_FONT);
	addChild(header);

	content = new pathos::Label(contents[selectedContent].c_str());
	content->setFont(LABEL_FONT);
	content->setX(320.0f);
	addChild(content);

	changeChoice(0);
}

ChoiceControl::~ChoiceControl() {
	delete header;
	delete content;
}

void ChoiceControl::changeChoice(int32 delta) {
	int32 maxIndex = (int32)(contents.size()) - 1;
	selectedContent = std::max(0, std::min(maxIndex, selectedContent + delta));
	std::wstring txt = L"<- " + contents[selectedContent] + L" ->";
	content->setText(txt.c_str());
}

// ------------------------------------------------------------------
// GameOptionsWidget

GameOptionsWidget::GameOptionsWidget(World_RacingTitle* inOwnerWorld)
	: BaseWidget()
{
	ownerWorld = inOwnerWorld;
	createUI();
	bindInput();
}

void GameOptionsWidget::createUI() {
	const int32 screenWidth = gEngine->getConfig().windowWidth;
	const int32 screenHeight = gEngine->getConfig().windowHeight;

	const float marginX = 60.0f, marginY = 60.0f;
	background = new pathos::Rectangle(screenWidth - marginX, screenHeight - marginY);
	background->setBrush(new SolidColorBrush(0.1f, 0.1f, 0.1f, 0.9f));
	background->setX(0.5f * marginX);
	background->setY(0.5f * marginY);
	addChild(background);

	std::vector<std::wstring> resolutions;
	for (auto i = 0u; i < NUM_RESOLUTIONS; ++i) {
		wchar_t resolutionMsg[128];
		auto& preset = RESOLUTION_PRESETS[i];
		swprintf_s(resolutionMsg, L"%d x %d", preset[0], preset[1]);
		resolutions.push_back(resolutionMsg);
	}
	resolutionControl = new ChoiceControl(L"Resolution", resolutions);
	resolutionControl->setXY(20.0f, 20.0f);
	background->addChild(resolutionControl);

	std::vector<std::wstring> screenModes{ L"Windowed", L"Fullscreen" };
	fullscreenControl = new ChoiceControl(L"Window Mode", screenModes);
	fullscreenControl->setXY(20.0f, resolutionControl->getY() + 80.0f);
	background->addChild(fullscreenControl);

	backToTitleLabel = new pathos::Label(L"Exit");
	backToTitleLabel->setFont(LABEL_FONT);
	backToTitleLabel->setXY(20.0f, background->getHeight() - 80.0f);
	background->addChild(backToTitleLabel);

	applyLabel = new pathos::Label(L"Apply");
	applyLabel->setFont(LABEL_FONT);
	applyLabel->setXY(20.0f, backToTitleLabel->getY() - 80.0f);
	background->addChild(applyLabel);

	optionItems = { resolutionControl->getHeaderLabel(), fullscreenControl->getHeaderLabel(), applyLabel, backToTitleLabel };
	selectedItem = 0;
	updateUI();
}

void GameOptionsWidget::bindInput() {
	ButtonBinding up({ InputConstants::KEYBOARD_ARROW_UP, InputConstants::XBOXONE_DPAD_UP });
	ButtonBinding down({ InputConstants::KEYBOARD_ARROW_DOWN, InputConstants::XBOXONE_DPAD_DOWN });
	ButtonBinding left({ InputConstants::KEYBOARD_ARROW_LEFT, InputConstants::XBOXONE_DPAD_LEFT });
	ButtonBinding right({ InputConstants::KEYBOARD_ARROW_RIGHT, InputConstants::XBOXONE_DPAD_RIGHT });
	ButtonBinding confirm({ InputConstants::SPACE, InputConstants::XBOXONE_A });

	auto handleChoiceControl = [this](pathos::Label* label, ChoiceControl* choice, int32 delta) {
		if (label == choice->getHeaderLabel()) {
			choice->changeChoice(delta);
			updateUI();
		}
	};

	inputManager->bindButtonPressed("up", up, [this]() {
		selectedItem = (selectedItem - 1 + (int32)optionItems.size()) % optionItems.size();
		updateUI();
	});
	inputManager->bindButtonPressed("down", down, [this]() {
		selectedItem = (selectedItem + 1) % optionItems.size();
		updateUI();
	});
	inputManager->bindButtonPressed("left", left, [this, handleChoiceControl]() {
		handleChoiceControl(optionItems[selectedItem], resolutionControl, -1);
		handleChoiceControl(optionItems[selectedItem], fullscreenControl, -1);
	});
	inputManager->bindButtonPressed("right", right, [this, handleChoiceControl]() {
		handleChoiceControl(optionItems[selectedItem], resolutionControl, +1);
		handleChoiceControl(optionItems[selectedItem], fullscreenControl, +1);
	});
	inputManager->bindButtonPressed("confirm", confirm, [this]() {
		if (optionItems[selectedItem] == applyLabel) {
			this->applyCurrentOptions();
		}
		else if (optionItems[selectedItem] == backToTitleLabel) {
			ownerWorld->onCloseOptionsWidget();
		}
	});
}

void GameOptionsWidget::updateUI() {
	auto handleChoiceControl = [](pathos::Label* label, ChoiceControl* control, const vector3& color) {
		if (label == control->getHeaderLabel()) {
			control->getContentLabel()->setColor(color);
		}
	};

	for (auto i = 0; i < optionItems.size(); ++i) {
		if ((int32)i == selectedItem) {
			optionItems[i]->setColor(LABEL_COLOR_FOCUS);
			handleChoiceControl(optionItems[i], resolutionControl, LABEL_COLOR_FOCUS);
			handleChoiceControl(optionItems[i], fullscreenControl, LABEL_COLOR_FOCUS);
		} else {
			optionItems[i]->setColor(LABEL_COLOR);
			handleChoiceControl(optionItems[i], resolutionControl, LABEL_COLOR);
			handleChoiceControl(optionItems[i], fullscreenControl, LABEL_COLOR);
		}
	}
}

void GameOptionsWidget::applyCurrentOptions() {
	int32 selectedResolution = resolutionControl->getChoice();
	CHECK(0 <= selectedResolution && selectedResolution < NUM_RESOLUTIONS);
	uint32 newResolutionX = (uint32)RESOLUTION_PRESETS[selectedResolution][0];
	uint32 newResolutionY = (uint32)RESOLUTION_PRESETS[selectedResolution][1];

	// #wip: Screen goes black if smaller than certain size.
	// Only if using auto exposure with luminance histogram :o
	// Maybe memory barrier problem?
	// rwLuminance results in nan (auto_exposure_histogram_avg.glsl)
	gEngine->getMainWindow()->setSize(newResolutionX, newResolutionY);

	int32 fullscreenChoice = fullscreenControl->getChoice();
	gEngine->getMainWindow()->setFullscreen(fullscreenChoice == 0 ? false : true);
}
