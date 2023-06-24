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
		GlobalFileLogger() {}
		GlobalFileLogger(const char* filename) {
			initialize(filename);
		}
		~GlobalFileLogger() {
			handle.close();
		}

		void initialize(const char* filename);

		void write(const char* data) {
			handle << data << '\n';
		}

		void flush() {
			handle.flush();
		}

		const std::string& getFilepath() const { return filepath; }

	private:
		std::string filepath;
		std::fstream handle;

	};

	extern GlobalFileLogger CommonLogFile;
	extern GlobalFileLogger ShaderLogFile;

}
