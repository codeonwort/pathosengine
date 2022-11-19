#pragma once

#include "pathos/scene/actor.h"
#include <string>

namespace pathos {

	class TextMeshComponent;

	class TextMeshActor : public Actor {

	public:
		TextMeshActor();

		void setText(const wchar_t* text);
		void setColor(float r, float g, float b);
		void setFont(const std::string& tag);

	private:
		TextMeshComponent* textComponent;

	};

}
