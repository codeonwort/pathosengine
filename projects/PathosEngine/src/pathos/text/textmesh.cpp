#include "textmesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/text/font_mgr.h"
#include "pathos/engine.h"
#include <iostream>

namespace pathos {

	TextMesh::TextMesh(string tag):Mesh(nullptr, nullptr) {
		this->tag = tag;
		doubleSided = true;
		glGenTextures(1, &texID);
		geometries.push_back(new PlaneGeometry(1, 1));
		auto mat = new TextureMaterial(texID);
		mat->setUseAlpha(true);
		mat->setLighting(false);
		materials.push_back(mat);
	}

	void TextMesh::setText(string txt, unsigned int rgb) {
		text = txt;
		auto set = FontManager::getFaceMap(tag);

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
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, totalWidth, totalHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, totalWidth, totalHeight, GL_RGBA, GL_UNSIGNED_BYTE, buf);
		delete buf;

		// TODO: remove the global access
		const EngineConfig& config = Engine::getConfig();
		float sx = (float)(2.0f / config.width) * totalWidth;
		float sy = (float)(2.0f / config.height) * totalHeight;
		transform.appendScale(glm::vec3(sx, -sy, 1));
	}

}