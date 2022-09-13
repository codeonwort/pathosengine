#pragma once

#include "display_object.h"
#include "pathos/text/text_geometry.h"
#include "pathos/text/font_mgr.h"
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

		virtual MeshGeometry* getGeometry() override { return geometry; }

	protected:
		virtual void updateTransform() override;

	private:
		std::wstring text;
		TextGeometry* geometry;
		FontDesc fontDesc;

	};

}
