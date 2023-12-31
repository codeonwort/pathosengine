#pragma once

// === Platform checklist ===
// [v] Windows  : Supported
// [ ] Linux    : No plan
// [ ] Android  : No plan
// [ ] Mac      : No plan
// [ ] iOS      : No plan

// ----------------------------------------------------------------------------
// Windows

#if (defined(_WIN32) && _WIN32) || (defined(_WIN64) && _WIN64)
	#define PLATFORM_WINDOWS 1
#endif

#ifndef PLATFORM_WINDOWS
	#define PLATFORM_WINDOWS 0
#endif

// ----------------------------------------------------------------------------
//
