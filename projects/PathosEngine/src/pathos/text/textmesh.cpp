#include "textmesh.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/text/font_mgr.h"
#include "pathos/engine.h"

#define DEBUG_FONT_CACHE 0
#define COMPACT_FONT_GLYPH 1
#define FLIP_GLYPH_V 0

namespace pathos {

	TextMesh::TextMesh(const std::string& tag_):Mesh(nullptr, nullptr) {
		tag = tag_;
		doubleSided = true;

		auto fontInfo = FontManager::getGlyphMap(tag);
		cache.init(FontManager::getFTLibrary(), fontInfo->filename.c_str(), fontInfo->fontSize);

		geometries.push_back(new TextGeometry);
		auto mat = new AlphaOnlyTextureMaterial(cache.getTexture());
		materials.push_back(mat);

#if DEBUG_FONT_CACHE
		geometries.push_back(new PlaneGeometry(1.0f, 1.0f));
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

		transform.identity();
		transform.appendScale(glm::vec3(1.0f, -1.0f, 1.0f));
	}

	void TextMesh::configureGeometry(const std::wstring& newText) {
		auto geom = static_cast<TextGeometry*>(geometries[0]);
		geom->configure(cache, newText);
	}

	void TextGeometry::configure(FontTextureCache& cache, const std::wstring& newText) {
		// TODO: prevent crash when newText is empty.

		positions.clear();
		uvs.clear();
		indices.clear();

		GLfloat penX = 0.0f;
		GLfloat penY = cache.getCellHeight();
		GLfloat penZ = 0.0f;
		GLfloat u, v, du, dv;
		GLuint idx = 0;

		cache.startGetGlyph();
		for (wchar_t x : newText) {
			if (x == L'\n') {
				penX = 0.0f;
				penY += cache.getCellHeight();
				continue;
			}
			const GlyphInTexture& glyph = cache.getGlyph(x);
			// order: top-left, top-right, bottom-right, bottom-left
			penY -= glyph.offsetY;

#if COMPACT_FONT_GLYPH
			const float dw = static_cast<float>(glyph.glyphWidth);
			const float dh = static_cast<float>(glyph.glyphHeight);
#else
			const float dw = cache.getCellWidth();
			const float dh = cache.getCellHeight();
#endif

			// v0
			positions.push_back(penX);
			positions.push_back(penY);
			positions.push_back(penZ);

			// v1
			positions.push_back(penX + dw);
			positions.push_back(penY);
			positions.push_back(penZ);

			// v2
			positions.push_back(penX + dw);
			positions.push_back(penY + dh);
			positions.push_back(penZ);

			// v3
			positions.push_back(penX);
			positions.push_back(penY + dh);
			positions.push_back(penZ);

			penY += glyph.offsetY;

			const float tw = static_cast<float>(cache.getTextureWidth());
			const float th = static_cast<float>(cache.getTextureHeight());

			u = glyph.x + (0.5f / tw);
			v = glyph.y + (0.5f / th);
#if COMPACT_FONT_GLYPH
			du = glyph.glyphWidth - (1.0f / tw);
			dv = glyph.glyphHeight - (1.0f / th);
#else
			du = glyph.width - (1.0f / tw);
			dv = glyph.height - (1.0f / th);
#endif

#if FLIP_GLYPH_V
			uvs.push_back(u); uvs.push_back(v + dv);
			uvs.push_back(u + du); uvs.push_back(v + dv);
			uvs.push_back(u + du); uvs.push_back(v);
			uvs.push_back(u); uvs.push_back(v);
#else
			uvs.push_back(u); uvs.push_back(v);
			uvs.push_back(u + du); uvs.push_back(v);
			uvs.push_back(u + du); uvs.push_back(v + dv);
			uvs.push_back(u); uvs.push_back(v + dv);
#endif

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

		updatePositionData(&positions[0], static_cast<uint32_t>(positions.size()));
		updateUVData(&uvs[0], static_cast<uint32_t>(uvs.size()));
		updateIndexData(&indices[0], static_cast<uint32_t>(indices.size()));

		if (normals.size() == 0) {
			for (auto i = 0u; i < newText.size(); ++i) {
				normals.push_back(0.0f);
				normals.push_back(0.0f);
				normals.push_back(1.0f);
			}
			updateNormalData(&normals[0], static_cast<uint32_t>(normals.size()));
		}
	}

}
