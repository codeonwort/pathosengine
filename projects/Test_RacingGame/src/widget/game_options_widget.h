#pragma once

#include "base_widget.h"

#include "pathos/overlay/display_object.h"
using namespace pathos;

#include <vector>

namespace pathos {
	class Label;
	class Rectangle;
}
class World_RacingTitle;

class ChoiceControl : public DisplayObject2D {
public:
	ChoiceControl(const std::wstring& label, const std::vector<std::wstring>& choices);
	~ChoiceControl();
	pathos::Label* getHeaderLabel() const { return header; }
	pathos::Label* getContentLabel() const { return content; }
	void changeChoice(int32 delta);
	int32 getChoice() const { return selectedContent; }
private:
	pathos::Label* header = nullptr;
	pathos::Label* content = nullptr;
	int32 selectedContent = -1;
	std::vector<std::wstring> contents;
};

class GameOptionsWidget : public BaseWidget {

public:
	GameOptionsWidget(World_RacingTitle* inOwnerWorld);

private:
	void createUI();
	void bindInput();
	void updateUI();

	void applyCurrentOptions();

	World_RacingTitle* ownerWorld = nullptr;

	pathos::Rectangle* background = nullptr;
	ChoiceControl* resolutionControl = nullptr;
	ChoiceControl* fullscreenControl = nullptr;
	pathos::Label* backToTitleLabel = nullptr;
	pathos::Label* applyLabel = nullptr;

	int32 selectedItem = -1;
	std::vector<pathos::Label*> optionItems;

};
