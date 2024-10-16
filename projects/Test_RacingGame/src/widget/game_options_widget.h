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
	pathos::Label* resolutionHeaderLabel = nullptr;
	pathos::Label* resolutionContentLabel = nullptr;
	
	pathos::Label* backToTitleLabel = nullptr;
	pathos::Label* applyLabel = nullptr;

	int32 selectedItem = -1;
	std::vector<pathos::Label*> optionItems;

	int32 selectedResolution = -1;

};
