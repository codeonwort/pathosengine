#include "label.h"
#include "pathos/text/font_mgr.h"
#include "pathos/overlay/brush.h"

namespace pathos {

	// #todo-text: Support multiple font types for Label instances
	static FontTextureCache g_labelFontTextureCache;
	static bool g_labelFontTextureCacheInitialized = false;

	Label::Label() {
		setName("label");

		if (!g_labelFontTextureCacheInitialized) {
			FontDesc fontDesc;
			bool validDesc = FontManager::get().getFontDesc("default", fontDesc);
			CHECK(validDesc);

			g_labelFontTextureCache.init(fontDesc.fullFilepath.c_str(), fontDesc.pixelSize);
			g_labelFontTextureCacheInitialized = true;
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
		geometry->configure(cmdList, g_labelFontTextureCache, text);
	}

	GLuint Label::getFontTexture() {
		return g_labelFontTextureCache.getTexture();
	}

	void Label::updateTransform() {
		transform.identity();
		transform.appendScale(scaleX, scaleY, 1.0f);
		transform.appendMove(x, y, 0.0f);
	}

}
