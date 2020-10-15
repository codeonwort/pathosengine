#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1
#endif

// Linux, Android -> Maybe?
// Mac, IOS       -> I don't have them
// Durango, Orbis -> I can't access them as a sole developer in my home :/

//////////////////////////////////////////////////////////////////////////

#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 0
#endif
