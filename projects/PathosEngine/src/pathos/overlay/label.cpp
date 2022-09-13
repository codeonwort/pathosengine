#include "label.h"
#include "brush.h"
#include "display_object_proxy.h"
#include "pathos/text/font_mgr.h"

// Fallback font that must exist.
#define DEFAULT_FONT_TAG    "default"

namespace pathos {

	Label::Label() {
		setName("label");
		setFont(DEFAULT_FONT_TAG);

		geometry = new TextGeometry;
		geometry->bCalculateLocalBounds = false;

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

	DisplayObject2DProxy* Label::createRenderProxy(OverlaySceneProxy* sceneProxy) {
		if (getVisible() && text.size() > 0) {
			updateTransform();

			LabelProxy* proxy = sceneProxy->allocate<LabelProxy>();
			proxy->x = x;
			proxy->y = y;
			proxy->scaleX = scaleX;
			proxy->scaleY = scaleY;
			proxy->geometry = geometry;
			proxy->brush = getBrush();
			proxy->transform = transform;
			proxy->text = text;
			proxy->fontDesc = fontDesc;
			return proxy;
		}
		return nullptr;
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

	void Label::updateTransform() {
		transform.identity();
		transform.appendScale(scaleX, scaleY, 1.0f);
		transform.appendMove(x, y, 0.0f);
	}

}
