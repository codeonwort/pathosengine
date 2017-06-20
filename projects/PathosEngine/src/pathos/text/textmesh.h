#pragma once

#include "pathos/mesh/mesh.h"

#include <GL/glew.h>

#include <string>
#include <map>

namespace pathos {

	class TextMesh : public Mesh {
	private:

		string tag, text;
		GLuint texID;
		//Transform* transform; // owner game object's transform
	public:
		//TextMesh(string tag, Transform* ownerTransform);
		TextMesh(string tag);
		void setText(string txt, unsigned int rgb);
	};

}