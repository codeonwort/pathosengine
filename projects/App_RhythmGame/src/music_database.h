#pragma once

#include "badger/types/int_types.h"

#include <string>
#include <vector>

struct MusicDatabaseItem {
	std::string title;
	std::string musicFile;
	std::string recordFile;
	std::string backgroundFile;
};

struct MusicDatabase {
	inline void addItem(const MusicDatabaseItem& item) {
		items.push_back(item);
	}

	inline uint32 numItems() const { return (uint32)items.size(); }

	std::vector<MusicDatabaseItem> items;
};

// Returns false if failed to read the database.
bool loadMusicDatabase(const char* filepath, const char* overridePath, MusicDatabase& outDB);
