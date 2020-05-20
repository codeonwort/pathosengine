#pragma once

#include <assert.h>

void CHECK_IMPL(int x, const char* file, int line);
void CHECKF_IMPL(int x, const char* msg, const char* file, int line);

#ifndef CHECK
	#if defined(_DEBUG)
		#define CHECK(x) assert(x)
	#else
		#define CHECK(x) CHECK_IMPL(!!(x), __FILE__, __LINE__)
	#endif
#endif

#ifndef CHECKF
	#if defined(_DEBUG)
		#define CHECKF(x, msg) assert((x) && msg);
	#else
		#define CHECKF(x, msg) CHECKF_IMPL(!!(x), msg, __FILE__, __LINE__)
	#endif
#endif

#define CHECK_NO_ENTRY() CHECK(false)
