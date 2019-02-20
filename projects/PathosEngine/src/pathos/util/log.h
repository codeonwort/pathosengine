#pragma once

#include <iostream>
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

#if ENABLE_LOGGER
	inline void LOG(LogSeverity severity, const char* format...) {
		printf("%s", severity_strings[(int)severity]);
		va_list argptr;
		va_start(argptr, format);
		vfprintf(stderr, format, argptr);
		va_end(argptr);
		puts("");
	}
#else
	inline void LOG(LogSeverity severity, const char* format...) {}
#endif

}