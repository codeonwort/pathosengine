#include "label.h"
#include "pathos/text/font_mgr.h"
#include "pathos/overlay/brush.h"

// Fallback font that must exist.
#define DEFAULT_FONT_TAG    "default"

namespace pathos {

	Label::Label() {
		setName("label");
		setFont(DEFAULT_FONT_TAG);

		geometry = new TextGeometry;

		setBrush(new TextBrush(1.0f, 1.0f, 1.0f));
		setScaleX(300.0f);
		setScaleY(300.0f);
	}

	Label::Label(const wchar_t* text)
		: Label()
	{
		setText(text);
	}

	Label::~Label() {
		delete geometry;
	}

	void Label::setText(const wchar_t* newText) {
		text = newText;
	}

	void Label::setColor(const vector3& newColor) {
		static_cast<TextBrush*>(getBrush())->setColor(newColor);
	}

	void Label::setFont(const std::string& tag) {
		bool validDesc = FontManager::get().getFontDesc(tag, fontDesc);
		if (!validDesc && tag != DEFAULT_FONT_TAG) {
			validDesc = FontManager::get().getFontDesc(DEFAULT_FONT_TAG, fontDesc);
		}
		CHECK(validDesc);
	}

	bool Label::onRender(RenderCommandList& cmdList) {
		if (text.size() > 0) {
			geometry->configure(cmdList, *fontDesc.cacheTexture, text);
			return true;
		}
		return false;
	}

	GLuint Label::getFontTexture() {
		return fontDesc.cacheTexture->getTexture();
	}

	void Label::updateTransform() {
		transform.identity();
		transform.appendScale(scaleX, scaleY, 1.0f);
		transform.appendMove(x, y, 0.0f);
	}

}
