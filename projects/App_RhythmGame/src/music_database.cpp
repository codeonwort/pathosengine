#include "music_database.h"

#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"
using namespace pathos;

bool loadMusicDatabase(const char* filepath, MusicDatabase& outDB)
{
	std::string dbPath = ResourceFinder::get().find(filepath);
	if (dbPath.size() == 0) {
		LOG(LogError, "Failed to find: %s", filepath);
		return false;
	}
	std::fstream db(dbPath, std::ios::in);
	if (!db) {
		LOG(LogError, "Failed to open: %s", dbPath.c_str());
		return false;
	}
	std::string rawLine;
	uint32 phase = 0;
	std::string title, musicFile, recordFile, backgroundFile;
	while (std::getline(db, rawLine)) {
		if (rawLine.size() == 0 || rawLine[0] == '#') {
			continue;
		}
		if (phase == 0) title = rawLine;
		else if (phase == 1) musicFile = rawLine;
		else if (phase == 2) recordFile = rawLine;
		else if (phase == 3) backgroundFile = rawLine;

		if (phase == 3) {
			outDB.addItem({ title, musicFile, recordFile, backgroundFile });
			phase = 0;
		} else {
			++phase;
		}
	}
	return true;
}
