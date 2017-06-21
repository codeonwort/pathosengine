#pragma once

#include "pathos/mesh/mesh.h"

#include <GL/glew.h>

#include <string>
#include <map>

namespace pathos {

	class TextMesh : public Mesh {
	private:

		std::string tag;
		std::wstring text;
		GLuint texID;
		//Transform* transform; // owner game object's transform
	public:
		//TextMesh(string tag, Transform* ownerTransform);
		TextMesh(const std::string& tag);
		void setText(const std::string& txt, unsigned int rgb); // CAUTION: ASCII-only! MBCS is not supported.
		void setText(const std::wstring& txt, unsigned int rgb);
	};

}