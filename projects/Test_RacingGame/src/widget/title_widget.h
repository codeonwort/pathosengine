#pragma once

#include "base_widget.h"

#include "pathos/overlay/display_object.h"
using namespace pathos;

#include <vector>

namespace pathos {
	class Label;
}
class World_RacingTitle;

class TitleWidget : public BaseWidget {

public:
	TitleWidget(World_RacingTitle* inOwnerWorld);

private:
	void createUI();
	void bindInput();
	void updateUI();

	World_RacingTitle* ownerWorld = nullptr;

	Label* startLabel = nullptr;
	Label* optionsLabel = nullptr;
	Label* exitLabel = nullptr;

	int32 selectedLabel = -1;
	std::vector<Label*> labels;

};
