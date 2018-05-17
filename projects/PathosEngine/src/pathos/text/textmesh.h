#pragma once

#include <string>
#include <map>

#include "GL/gl_core_4_3.h"

#include "pathos/mesh/mesh.h"
#include "pathos/text/font_texture_cache.h"

namespace pathos {

	class TextGeometry : public MeshGeometry {
	public:
		void configure(FontTextureCache& cache, const std::wstring& text);
	private:
		std::vector<GLfloat> positions;
		std::vector<GLfloat> normals;
		std::vector<GLfloat> uvs;
		std::vector<GLuint> indices;
	};

	class TextMesh : public Mesh {

	public:
		//TextMesh(string tag, Transform* ownerTransform);
		TextMesh(const std::string& tag);
		void setText(const std::string& txt, unsigned int rgb); // CAUTION: ASCII-only! MBCS is not supported.
		void setText(const std::wstring& txt, unsigned int rgb);

	private:
		void configureGeometry(const std::wstring& text);

		// TODO: why is this a member variable?
		//       make it shared by text meshes.
		FontTextureCache cache;

		std::string tag;
		std::wstring text;
		//Transform* transform; // owner game object's transform

	};

}