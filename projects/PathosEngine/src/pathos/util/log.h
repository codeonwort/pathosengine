#pragma once

#include "file_system.h"
#include "string_conversion.h"

#include <fstream>
#include <stdarg.h>

#define ENABLE_LOGGER 1

namespace pathos {

	enum LogSeverity {
		LogDebug   = 0,
		LogInfo    = 1,
		LogWarning = 2,
		LogError   = 3,
		LogFatal   = 4
	};

	// #todo-log: Log category, log window
	// Wrtie a log in the console window and the global log file.
	void LOG(LogSeverity severity, const char* format...);

	// Write logs to a file. Not thread-safe.
	struct LogFileWriter
	{
		LogFileWriter();
		~LogFileWriter();

		/// <summary>
		/// Create a log file in the '[solution_dir]/log/' folder.
		/// </summary>
		/// <param name="filename">Filename with an extension.</param>
		/// <param name="renameOldFileToKeep">If true and given filename already exists, then rename the old file so that it's not overwritten.
		/// For example, given "asd.txt", the old file is renamed to "asd.old.N.txt" where N is the smallest positive integer
		/// that does not make the new name conflicts with other filenames in the same folder.
		/// If false, then the file will be overwritten.</param>
		void initialize(const char* filename, bool renameOldFileToKeep = true);

		// Wrtie a line to the file. Automatically appends a newline character.
		void writeLine(const char* line);

		// Writing to file could be pending until flushed.
		void flush();

		// Same as consecutive calls of write() and flush().
		void writeLineAndFlush(const char* line);

		const std::string& getFilepath() const { return filepath; }

	private:
		std::string filepath;
		std::fstream fileHandle;

	};
	
	extern LogFileWriter gGlobalLogFile;

}
