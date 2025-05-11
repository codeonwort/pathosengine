#pragma once

#include "display_object.h"
#include "pathos/text/text_geometry.h"
#include "pathos/text/font_mgr.h"
#include "pathos/smart_pointer.h"

#include <string>

namespace pathos {

	class Label : public DisplayObject2D {

	public:
		Label();
		Label(const wchar_t* text);
		~Label();

		DisplayObject2DProxy* createRenderProxy(OverlaySceneProxy* sceneProxy) override;

		void setText(const wchar_t* newText);
		void setColor(const vector3& newColor);
		void setFont(const std::string& tag);

		uint32 getTextWidth() const;

		const std::wstring& getText() const { return text; }
		std::wstring getText() { return text; }

	protected:
		virtual void updateTransform(uint32 viewportWidth, uint32 viewportHeight) override;

	private:
		std::wstring text;
		assetPtr<TextGeometry> geometry;
		FontDesc fontDesc;

	};

}
