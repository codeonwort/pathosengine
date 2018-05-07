#include "textmesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/text/font_mgr.h"
#include "pathos/engine.h"

#define DEBUG_TEXT 0

namespace pathos {

	TextMesh::TextMesh(const std::string& tag_):Mesh(nullptr, nullptr) {
		tag = tag_;
		doubleSided = true;

		auto fontInfo = FontManager::getGlyphMap(tag);
		cache.init(FontManager::getFTLibrary(), fontInfo->filename.c_str(), fontInfo->fontSize);

		geometries.push_back(new MeshGeometry);
		auto mat = new AlphaOnlyTextureMaterial(cache.getTexture());
		materials.push_back(mat);

#if DEBUG_TEXT
		geometries.push_back(new PlaneGeometry(1, 1));
		materials.push_back(mat);
#endif
	}

	// CAUTION: ASCII-only! MBCS is not supported.
	void TextMesh::setText(const std::string& txt, unsigned int rgb) {
		std::wstring w(txt.begin(), txt.end());
		setText(w, rgb);
	}

	void TextMesh::setText(const std::wstring& txt, unsigned int rgb) {
		text = txt;
		configureGeometry(text);

		GLfloat r = static_cast<float>(rgb >> 16) / 255.0f;
		GLfloat g = static_cast<float>((rgb >> 8) & 0xff) / 255.0f;
		GLfloat b = static_cast<float>(rgb & 0xff) / 255.0f;
		static_cast<AlphaOnlyTextureMaterial*>(materials[0])->setColor(r, g, b);

		// Calculate appropriate texture size
		/*auto set = FontManager::getGlyphMap(tag);
		int totalWidth = 0;
		int totalHeight = set->maxHeight + 8; // meh
		for (auto it = text.begin(); it != text.end(); it++) {
			auto& g = set->getGlyphCache(*it);
			totalWidth += g.advance.x >> 6;
		}
		totalWidth = (int)ceil(totalWidth / 4.0f + 0.5f) * 4;
		totalHeight = (int)ceil(totalHeight / 4.0f + 0.5f) * 4;

		// TODO: remove the global access
		const EngineConfig& config = Engine::getConfig();
		float sx = (float)(2.0f / config.width) * totalWidth;
		float sy = (float)(2.0f / config.height) * totalHeight;*/
		transform.identity();
		transform.appendScale(glm::vec3(1.0f, -1.0f, 1.0f));
	}

	/*
	void TextMesh::setText(const std::wstring& txt, unsigned int rgb) {
		text = txt;
		auto set = FontManager::getGlyphMap(tag);

		// Calculate appropriate texture size
		int totalWidth = 0;
		int totalHeight = set->maxHeight + 8; // meh
		for (auto it = text.begin(); it != text.end(); it++) {
			auto& g = set->getGlyphCache(*it);
			totalWidth += g.advance.x >> 6;
		}
		totalWidth = (int)ceil(totalWidth / 4.0f + 0.5f) * 4;
		totalHeight = (int)ceil(totalHeight / 4.0f + 0.5f) * 4;

		glBindTexture(GL_TEXTURE_2D, texID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, totalWidth, totalHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Fill background color
		unsigned char red = (rgb >> 16) & 0xFF;
		unsigned char green = (rgb >> 8) & 0xFF;
		unsigned char blue = rgb & 0xFF;
		const int bufSize = totalWidth * totalHeight * 4;
		GLubyte *buf = new GLubyte[bufSize];
		for (int i = 0; i < bufSize; i += 4) {
			buf[i] = red; buf[i + 1] = green; buf[i + 2] = blue; buf[i + 3] = 0;
		}
		
		// Draw text to the texture
		if (text.length() > 0) {
			int x = 0;
			auto& head = set->getGlyphCache(text[0]);
			int y = head.bitmap.rows;
			int dy;
			for (auto it = text.begin(); it != text.end(); ++it) {
				auto& g = set->getGlyphCache(*it);
				dy = g.bitmap.rows - g.bitmap_top;
				//dy = totalHeight
				for (int ty = 0; ty < g.bitmap.rows; ++ty) {
					int p = 4 * (((y - g.bitmap.rows) + ty + dy) * totalWidth + x) + 3;
					for (int tx = 0; tx < g.bitmap.width; ++tx) {
						buf[p] = g.bitmap.buffer[(ty * g.bitmap.width) + tx];
						p += 4;
					}
				}
				//glTexSubImage2D(GL_TEXTURE_2D, 0, x, y+(totalHeight-g->bitmap.rows), g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
				x += g.advance.x >> 6;
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
	*/

	void TextMesh::configureGeometry(const std::wstring& newText) {
		positions.clear();
		uvs.clear();
		indices.clear();

		GLfloat penX = 0.0f, penY = 0.0f, penZ = 0.0f;
		GLfloat u, v, du, dv;
		GLuint idx = 0;

		cache.startGetGlyph();
		for (wchar_t x : newText) {
			if (x == L'\n') {
				penX = 0.0f;
				penY += cache.getCellHeight();
			}
			const GlyphInTexture& glyph = cache.getGlyph(x);
			// order: top-left, top-right, bottom-right, bottom-left
			penY -= glyph.offsetY;
			positions.push_back(penX); positions.push_back(penY); positions.push_back(penZ);
			positions.push_back(penX + cache.getCellWidth()); positions.push_back(penY); positions.push_back(penZ);
			positions.push_back(penX + cache.getCellWidth()); positions.push_back(penY + cache.getCellHeight()); positions.push_back(penZ);
			positions.push_back(penX); positions.push_back(penY + cache.getCellHeight()); positions.push_back(penZ);
			penY += glyph.offsetY;
			penZ += 0.00001f;

			u = glyph.x + (0.5f / cache.getTextureWidth());
			v = glyph.y + (0.5f / cache.getTextureHeight());
			du = glyph.width - (0.5f / cache.getTextureWidth());
			dv = glyph.height - (0.5f / cache.getTextureHeight());
			uvs.push_back(u); uvs.push_back(v);
			uvs.push_back(u + du); uvs.push_back(v);
			uvs.push_back(u + du); uvs.push_back(v + dv);
			uvs.push_back(u); uvs.push_back(v + dv);

			indices.push_back(idx);
			indices.push_back(idx + 2);
			indices.push_back(idx + 1);
			indices.push_back(idx);
			indices.push_back(idx + 3);
			indices.push_back(idx + 2);

			if (x != L'\n') {
				penX += glyph.advanceX;
			}
			idx += 4;
		}
		cache.endGetGlyph();

		MeshGeometry* geom = geometries[0];
		geom->updateVertexData(&positions[0], positions.size());
		geom->updateUVData(&uvs[0], uvs.size());
		geom->updateIndexData(&indices[0], indices.size());

		if (normals.size() == 0) {
			for (auto i = 0u; i < newText.size(); ++i) {
				normals.push_back(0.0f); normals.push_back(0.0f); normals.push_back(1.0f);
			}
			geom->updateNormalData(&normals[0], normals.size());
		}
	}

}