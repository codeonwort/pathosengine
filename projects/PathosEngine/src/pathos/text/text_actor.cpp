#include "text_actor.h"
#include "pathos/text/text_component.h"

namespace pathos {

	TextMeshActor::TextMeshActor() {
		textComponent = createDefaultComponent<TextMeshComponent>();
		setAsRootComponent(textComponent);
	}

	void TextMeshActor::setText(const wchar_t* text) {
		textComponent->setText(text);
	}

	void TextMeshActor::setColor(float r, float g, float b) {
		textComponent->setColor(r, g, b);
	}

	void TextMeshActor::setFont(const std::string& tag) {
		textComponent->setFont(tag);
	}

}
