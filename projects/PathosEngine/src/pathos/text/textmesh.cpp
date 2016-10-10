#include <pathos/text/textmesh.h>
#include <pathos/engine.h>
#include <iostream>

namespace pathos {

	FT_Library TextMesh::library;
	std::map<std::string, FaceMap*> TextMesh::fontSet;
	bool TextMesh::textMeshReady = false;
	void TextMesh::textMeshInit() {
		if (textMeshReady) return;

		// init free type and load fonts
		if (FT_Init_FreeType(&library)) {
			std::cerr << "** Cannot init freetype library **" << std::endl;
			return;
		}
		loadFont("default", "../../resources/fonts/consola.ttf", 16);

		textMeshReady = true;
	}
	void TextMesh::loadChar(FT_Face* face, char x, const char* name, unsigned int size) {
		if (FT_New_Face(library, name, 0, face)) {
			std::cerr << "** Cannot open the font file" << std::endl;
			return;
		}
		FT_Set_Pixel_Sizes(*face, 0, size);
		if (FT_Load_Char(*face, x, FT_LOAD_RENDER)) {
			std::cerr << "** Cannot load the character: " << x << std::endl;
			return;
		}
	}
	void TextMesh::loadFont(std::string tag, const char* name, unsigned int size) {
		FaceMap* map = new FaceMap;
		FT_Face face;
		string ary = " ./";
		for (char x = 'a'; x <= 'z'; x++) ary += x;
		for (char x = 'A'; x <= 'Z'; x++) ary += x;
		for (char x = '0'; x <= '9'; x++) ary += x;
		for (auto it = ary.begin(); it != ary.end(); it++) {
			char x = *it;
			loadChar(&face, x, name, size);
			map->insert(std::pair<char, FT_Face>(x, face));
		}
		fontSet.insert(std::pair<std::string, FaceMap*>(tag, map));
	}

	TextMesh::TextMesh(string tag):Mesh(nullptr, nullptr) {
		TextMesh::textMeshInit();
		this->tag = tag;
		doubleSided = true;
		glGenTextures(1, &texID);
		geometries.push_back(new PlaneGeometry(1, 1));
		materials.push_back(make_shared<TextureMaterial>(texID, true));
	}

	void TextMesh::setText(string txt, unsigned int rgb) {
		text = txt;
		FaceMap* set = fontSet.find(tag)->second;

		// Calculate appropriate texture size
		int totalWidth = 0, totalHeight = 0;
		for (auto it = text.begin(); it != text.end(); it++) {
			FT_GlyphSlot g = set->find(*it)->second->glyph;
			totalWidth += g->advance.x >> 6;
			if (g->bitmap.rows > totalHeight) {
				totalHeight = g->bitmap.rows;
			}
		}
		totalWidth = (int)ceil(totalWidth / 4.0f + 0.5f) * 4;
		totalHeight = (int)ceil(totalHeight / 4.0f + 0.5f) * 4;
		totalHeight += 8;

		glBindTexture(GL_TEXTURE_2D, texID);
		glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA, totalWidth, totalHeight);

		// Fill background color
		unsigned char red = (rgb >> 16) & 0xFF;
		unsigned char green = (rgb >> 8) & 0xFF;
		unsigned char blue = rgb & 0xFF;
		GLubyte *buf = new GLubyte[totalWidth * totalHeight * 4];
		for (int i = 0; i < totalWidth*totalHeight*4; i+=4){
			buf[i] = red; buf[i + 1] = green; buf[i + 2] = blue; buf[i + 3] = 0;
		}
		
		// Draw text to the texture
		if (text.length() > 0) {
			int x = 0, y = set->find(text[0])->second->glyph->bitmap.rows + 4, dy;
			for (auto it = text.begin(); it != text.end(); it++) {
				FT_GlyphSlot g = set->find(*it)->second->glyph;
				dy = g->bitmap.rows - g->bitmap_top;
				//dy = totalHeight
				for (int ty = 0; ty < g->bitmap.rows; ty++) {
					int p = 4 * (((y - g->bitmap.rows) + ty + dy) * totalWidth + x) + 3;
					for (int tx = 0; tx < g->bitmap.width; tx++) {
						buf[p] = g->bitmap.buffer[(ty * g->bitmap.width) + tx];
						p += 4;
					}
				}
				//glTexSubImage2D(GL_TEXTURE_2D, 0, x, y+(totalHeight-g->bitmap.rows), g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
				x += g->advance.x >> 6;
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, totalWidth, totalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
		delete buf;

		const EngineConfig& config = Engine::getConfig();
		float sx = (float)(2.0f / config.width) * totalWidth;
		float sy = (float)(2.0f / config.height) * totalHeight;
		transform.appendScale(glm::vec3(sx, -sy, 1));
	}

}