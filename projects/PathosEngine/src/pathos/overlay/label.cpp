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
		setScaleX(10.0f);
		setScaleY(10.0f);
	}

	Label::Label(const wchar_t* text) : Label() {
		setText(text);
	}

	Label::~Label() {
		delete geometry;
	}

	void Label::setText(const wchar_t* newText) {
		geometry->configure(cache, newText);
	}

	GLuint Label::getFontTexture() {
		return cache.getTexture();
	}

	void Label::updateTransform() {
		transform.identity();
		transform.appendMove(0.5f, 0.5f, 0.0f);
		transform.appendScale(100 * scaleX, 100 * -scaleY, 1.0f);
		transform.appendMove(x, y, 0.0f);
		transform.appendScale(1.0f, -1.0f, 1.0f);
	}

}