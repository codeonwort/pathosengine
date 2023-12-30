#include "log.h"

#include <mutex>
#include <iostream>
#include <filesystem>
#include <stdio.h>

namespace pathos {

	static const char* severity_strings[] = {
		"[DEBUG] ",
		"[INFO] ",
		"[WARNING] ",
		"[ERROR] ",
		"[FATAL] "
	};

	static std::mutex gLogMutex;
	LogFileWriter gGlobalLogFile;

	void LOG(LogSeverity severity, const char* format...) {
#if ENABLE_LOGGER
		// #todo-log: Bad mutex for just logging
		std::lock_guard<std::mutex> guard(pathos::gLogMutex);

		va_list argptr;
		va_start(argptr, format);
		int n = std::vsnprintf(nullptr, 0, format, argptr);
		va_end(argptr);
		va_start(argptr, format);
		char* buf = new char[n + 1];
		std::vsnprintf(buf, n + 1, format, argptr);
		va_end(argptr);

		printf("%s%s\n", severity_strings[(int)severity], buf);
		gGlobalLogFile.writeLineAndFlush(buf);
		if (severity == LogFatal) {
			__debugbreak();
		}

		delete[] buf;
#endif
	}

	LogFileWriter::LogFileWriter() {
	}

	LogFileWriter::~LogFileWriter() {
		fileHandle.close();
	}

	void LogFileWriter::initialize(const char* filename, bool renameOldFileToKeep) {
		std::string logDir = pathos::getSolutionDir();
		logDir += "log/";
		pathos::createDirectory(logDir.c_str());

		std::string stem = filename, ext = "";
		size_t ix = stem.find_last_of('.');
		if (ix != std::string::npos) {
			ext = stem.substr(ix);
			stem = stem.substr(0, ix);
		}

		std::string filepath = logDir + stem + ext;
		if (renameOldFileToKeep && std::filesystem::exists(filepath)) {
			int32 oldNumber = 0;
			std::string oldPath; // Renamed path for previous log file
			do {
				oldPath = logDir + stem + ".old." + std::to_string(++oldNumber) + ext;
			} while (std::filesystem::exists(oldPath));
			std::filesystem::rename(filepath, oldPath);
		}

		fileHandle.open(filepath, std::ios::out | std::ios::trunc);
		if (fileHandle.is_open()) {
			LOG(LogDebug, "Initialize LogFileWriter: %s", filepath.data());
		} else {
			LOG(LogError, "Failed to create LogFileWriter: %s", filepath.data());
		}
	}

	void LogFileWriter::writeLine(const char* line) {
		fileHandle << line << '\n';
	}

	void LogFileWriter::flush() {
		fileHandle.flush();
	}

	void LogFileWriter::writeLineAndFlush(const char* line) {
		fileHandle << line << std::endl;
	}

}
