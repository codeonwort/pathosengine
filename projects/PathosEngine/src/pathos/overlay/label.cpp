#include "label.h"
#include "pathos/text/font_mgr.h"
#include "pathos/overlay/brush.h"

namespace pathos {

	static FontTextureCache cache;
	static bool cacheInitialized = false;

	Label::Label() {
		setName("label");
		if (!cacheInitialized) {
			auto fontInfo = FontManager::getGlyphMap("default");
			cache.init(FontManager::getFTLibrary(), fontInfo->filename.c_str(), fontInfo->fontSize);
			cacheInitialized = true;
		}
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

	void Label::onRender(RenderCommandList& cmdList) {
		geometry->configure(cmdList, cache, text);
	}

	GLuint Label::getFontTexture() {
		return cache.getTexture();
	}

	void Label::updateTransform() {
		transform.identity();
		transform.appendScale(scaleX, scaleY, 1.0f);
		transform.appendMove(x, y, 0.0f);
	}

}
