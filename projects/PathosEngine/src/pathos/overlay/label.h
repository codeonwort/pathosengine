#pragma once

#include "display_object.h"
#include "pathos/text/textmesh.h"

namespace pathos {

	class Label : public DisplayObject2D {
	public:
		Label();
		Label(const wchar_t* text);
		~Label();
		void setText(const wchar_t* newText);
		GLuint getFontTexture();
		virtual MeshGeometry* getGeometry() override { return geometry; }
	protected:
		virtual void updateTransform() override;
	private:
		TextGeometry* geometry;
	};

}