#pragma once

#include "display_object.h"
#include "pathos/text/textmesh.h"

namespace pathos {

	class Label : public DisplayObject2D {

	public:
		static GLuint getFontTexture();

	public:
		Label();
		Label(const wchar_t* text);
		~Label();

		void setText(const wchar_t* newText);

		virtual MeshGeometry* getGeometry() override { return geometry; }

	protected:
		virtual void updateTransform() override;

	private:
		TextGeometry* geometry;

	};

}