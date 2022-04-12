#pragma once

#include "file_system.h"
#include "string_conversion.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdarg.h>

#define ENABLE_LOGGER 1

namespace pathos {

	static const char* severity_strings[] = {
		"[DEBUG] ",
		"[INFO] ",
		"[WARNING] ",
		"[ERROR] ",
		"[FATAL] "
	};

	enum LogSeverity {
		LogDebug   = 0,
		LogInfo    = 1,
		LogWarning = 2,
		LogError   = 3,
		LogFatal   = 4
	};

	void LOG(LogSeverity severity, const char* format...);

	struct GlobalFileLogger
	{
		GlobalFileLogger(const char* filename) {
			std::wstring w_exec_dir;
			getExecDir(w_exec_dir);

			std::string filepath;
			WCHAR_TO_MBCS(w_exec_dir, filepath);
			filepath += filename;

			handle.open(filepath, std::ios::out | std::ios::trunc);
			if (handle.is_open()) {
				LOG(LogDebug, "Initialize GlobalFileLogger: %s", filepath.data());
			} else {
				LOG(LogError, "Failed to create GlobalFileLogger: %s", filepath.data());
			}
		}
		~GlobalFileLogger() {
			handle.close();
		}

		void write(const char* data)
		{
			handle << data << '\n';
		}

	private:
		std::fstream handle;

	};

	extern GlobalFileLogger CommonLogFile;
	extern GlobalFileLogger ShaderLogFile;

}
