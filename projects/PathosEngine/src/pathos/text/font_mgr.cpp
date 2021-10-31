#include "font_mgr.h"
#include "pathos/text/font_texture_cache.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

#define REMOVE_GLYPH_LOADING 1

namespace pathos {

	FontManager& FontManager::get()
	{
		static FontManager instance;
		return instance;
	}
	
	FontManager::FontManager() {}
	FontManager::~FontManager() {
		if (initialized) {
			term();
		}
	}

	bool FontManager::init() {
		if (initialized) {
			return true;
		}

		if (FT_Init_FreeType(&library)) {
			LOG(LogError, "Cannot init FreeType library");
			return false;
		}
		FT_Int major, minor, patch;
		FT_Library_Version(library, &major, &minor, &patch);
		LOG(LogInfo, "[ThirdParty] FreeType : %d.%d.%d", major, minor, patch);

		initialized = true;
		return initialized;
	}

	bool FontManager::term() {
		if (!initialized) {
			LOG(LogError, "[ThirdParty] FontManager is even not initialized yet");
			return false;
		}
		// Release FreeType library
		if (FT_Done_FreeType(library)) {
			LOG(LogError, "[ThirdParty] Error while destroying FreeType library");
			return false;
		}
		return true;
	}

	bool FontManager::registerFont(const std::string& tag, const char* filepath, uint32 pixelSize) {
		if (fontDescDict.find(tag) != fontDescDict.end()) {
			LOG(LogError, "Tag is already registered: %s (filepath=%s, pixelSize=%u)", tag.c_str(), filepath, pixelSize);
			return false;
		}
		std::string fullFilepath = ResourceFinder::get().find(filepath);
		if (fullFilepath.size() == 0) {
			LOG(LogError, "Cannot find a font file: %s", filepath);
			return false;
		}

		FontDesc desc;
		desc.fullFilepath = fullFilepath;
		desc.pixelSize = pixelSize;
		desc.cacheTexture = new FontTextureCache;
		desc.cacheTexture->init(fullFilepath.c_str(), pixelSize);
		fontDescDict[tag] = std::move(desc);

		return true;
	}

	bool FontManager::getFontDesc(const std::string& tag, FontDesc& outDesc) {
		auto it = fontDescDict.find(tag);
		if (it == fontDescDict.end()) {
			return false;
		}
		outDesc = it->second;
		return true;
	}

	void FontManager::registerCache(FontTextureCache* cache)
	{
		cacheList.push_back(cache);
	}

	void FontManager::unregisterCache(FontTextureCache* cache)
	{
		cacheList.remove_if([cache](FontTextureCache* x) { return x == cache; });
	}

	void FontManager::onFrameEnd()
	{
		for (FontTextureCache* cache : cacheList) {
			cache->onFrameEnd();
		}
	}

}
