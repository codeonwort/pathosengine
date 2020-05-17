#pragma once

#include <assert.h>

#ifndef CHECK
	#define CHECK(x) assert(x)
#endif

#define CHECKF(x, msg) assert((x) && msg);

#define CHECK_NO_ENTRY() CHECK(false)
