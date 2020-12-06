#pragma once

#include "badger/types/noncopyable.h"
#include "badger/math/spline.h"
#include <string>

namespace pathos {

	class SplineLoader : public Noncopyable {

	public:
		SplineLoader() = default;
		~SplineLoader() = default;

		bool load(const char* inFilename, bool bMakeLoop, std::string& outName, HermiteSpline& outSpline);

	};

}
