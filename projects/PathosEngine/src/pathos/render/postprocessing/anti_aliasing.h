#pragma once

#include "badger/types/int_types.h"

namespace pathos {

	enum class EAntiAliasingMethod : uint8 {
		NoAA = 0, // Skip anti-aliasing pass
		FXAA = 1, // NVidia FXAA
		TAA = 2,
		NumMethods = 3
	};

	EAntiAliasingMethod getAntiAliasingMethod();

	float getTemporalJitterMultiplier();

}
