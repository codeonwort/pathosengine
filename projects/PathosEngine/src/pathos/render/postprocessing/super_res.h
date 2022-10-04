#pragma once

#include "pathos/console.h"

#include "badger/types/int_types.h"

namespace pathos {

	enum class ESuperResolutionMethod : uint8 {
		Disabled = 0,
		FSR1 = 1,
	};

	enum class EFSR1QualityMode : uint8 {
		UltraQuality,
		Quality,
		Balanced,
		Performance
	};

	ESuperResolutionMethod getSuperResolutionMethod();
	float getSuperResolutionScaleFactor();

}
