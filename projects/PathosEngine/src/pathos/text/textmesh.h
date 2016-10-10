#pragma once

#include <string>
#include <map>

#include <pathos/mesh/mesh.h>

#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace pathos;

namespace pathos {

	typedef std::map<char, FT_Face> FaceMap;

	class TextMesh : public Mesh {
	private:
		static FT_Library library;
		static std::map<std::string, FaceMap*> fontSet;
		static bool textMeshReady; // This should go to the engine init part
		static void textMeshInit(); // and this also
		static void loadChar(FT_Face* face, char x, const char* name, unsigned int size);
		static void loadFont(std::string tag, const char* name, unsigned int size);

		string tag, text;
		GLuint texID;
		//Transform* transform; // owner game object's transform
	public:
		//TextMesh(string tag, Transform* ownerTransform);
		TextMesh(string tag);
		void setText(string txt, unsigned int rgb);
	};

}