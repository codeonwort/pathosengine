#include "log.h"

#include <mutex>

namespace pathos {

	std::mutex logMutex;

	void LOG(LogSeverity severity, const char* format...) {
#if ENABLE_LOGGER
		// #todo: Bad mutex for just logging
		std::lock_guard<std::mutex> guard(pathos::logMutex);

		printf("%s", severity_strings[(int)severity]);
		va_list argptr;
		va_start(argptr, format);
		vfprintf(stderr, format, argptr);
		va_end(argptr);
		puts("");

		if (severity == LogFatal) {
			__debugbreak();
		}
#endif
	}

	void GlobalFileLogger::initialize(const char* filename) {
		std::wstring w_exec_dir;
		getExecDir(w_exec_dir);

		WCHAR_TO_MBCS(w_exec_dir, filepath);
		filepath += filename;

		handle.open(filepath, std::ios::out | std::ios::trunc);
		if (handle.is_open()) {
			LOG(LogDebug, "Initialize GlobalFileLogger: %s", filepath.data());
		} else {
			LOG(LogError, "Failed to create GlobalFileLogger: %s", filepath.data());
		}
	}

	GlobalFileLogger CommonLogFile("common_log.txt");
	GlobalFileLogger ShaderLogFile("shader_log.txt");

}
