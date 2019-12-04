#pragma once

#include <assert.h>

#ifndef CHECK
	#define CHECK(x) assert(x)
#endif

#define CHECK_NO_ENTRY() CHECK(false)
