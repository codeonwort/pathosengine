#pragma once

#include "pathos/mesh/mesh.h"
#include "pathos/text/font_texture_cache.h"

#include <GL/glew.h>

#include <string>
#include <map>

namespace pathos {

	class TextMesh : public Mesh {

	public:
		//TextMesh(string tag, Transform* ownerTransform);
		TextMesh(const std::string& tag);
		void setText(const std::string& txt, unsigned int rgb); // CAUTION: ASCII-only! MBCS is not supported.
		void setText(const std::wstring& txt, unsigned int rgb);

	private:
		void configureGeometry(const std::wstring& text);

		FontTextureCache cache;

		std::string tag;
		std::wstring text;
		//Transform* transform; // owner game object's transform

		std::vector<GLfloat> positions;
		std::vector<GLfloat> normals;
		std::vector<GLfloat> uvs;
		std::vector<GLuint> indices;

	};

}