#pragma once

#include "display_object.h"
#include "pathos/text/textmesh.h"
#include <string>

namespace pathos {

	class Label : public DisplayObject2D {

	public:
		static GLuint getFontTexture();

	public:
		Label();
		Label(const wchar_t* text);
		~Label();

		void setText(const wchar_t* newText);
		void setColor(const vector3& newColor);

		virtual void onRender(RenderCommandList& cmdList) override;

		virtual MeshGeometry* getGeometry() override { return geometry; }

	protected:
		virtual void updateTransform() override;

	private:
		std::wstring text;
		TextGeometry* geometry;

	};

}
