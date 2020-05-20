#include "assertion.h"
#include <stdio.h>

void CHECK_IMPL(int x, const char* file, int line) {
	static thread_local char buffer[2048];
	if (!x) {
		sprintf_s(buffer, "Assertion failed !!! [FILE=%s] [LINE=%d]\n", file, line);
		puts(buffer);
		__debugbreak();
	}
}

void CHECKF_IMPL(int x, const char* msg, const char* file, int line) {
	static thread_local char buffer[2048];
	if (!x) {
		sprintf_s(buffer, "Assertion failed !!! [MSG=%s] [FILE=%s] [LINE=%d]\n", msg, file, line);
		puts(buffer);
		__debugbreak();
	}
}
