#pragma once

#include <assert.h>

void CHECK_IMPL(int x, const char* file, int line);
void CHECKF_IMPL(int x, const char* msg, const char* file, int line);

#ifndef ASSERT
	#define ASSERT(x) assert(x)
#endif

#ifndef CHECK
	#define CHECK(x) CHECK_IMPL(!!(x), __FILE__, __LINE__)
#endif

#ifndef CHECKF
	#define CHECKF(x, msg) CHECKF_IMPL(!!(x), msg, __FILE__, __LINE__)
#endif

#define CHECK_NO_ENTRY() CHECK(false)
