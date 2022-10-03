#include "super_res.h"
#include "badger/math/minmax.h"

namespace pathos {

	ConsoleVariable<int32> cvar_super_res("r.super_res.method", 0, "0 = disabled, 1 = AMD FSR1");

	ConsoleVariable<int32> cvar_fsr1_quality("r.fsr1.quality", 0,
		"0 = ultra quality, 1 = quality, 2 = balanced, 3 = performance");

	float getFSR1ScaleFactor() {
		EFSR1QualityMode mode = (EFSR1QualityMode)badger::clamp(0, cvar_fsr1_quality.getInt(), 3);

		switch (mode) {
		case EFSR1QualityMode::UltraQuality: return 1.3f;
		case EFSR1QualityMode::Quality: return 1.5f;
		case EFSR1QualityMode::Balanced: return 1.7f;
		case EFSR1QualityMode::Performance: return 2.0f;
		default: CHECK_NO_ENTRY(); return 1.0f;
		}
		return 1.0f;
	}

}

namespace pathos {

	ESuperResolutionMethod getSuperResolutionMethod() {
		constexpr int32 numMethods = 1;
		return (ESuperResolutionMethod)badger::clamp(0, cvar_super_res.getInt(), numMethods);
	}

	float getSuperResolutionScaleFactor() {
		switch (getSuperResolutionMethod()) {
			case ESuperResolutionMethod::Disabled: return 1.0f;
			case ESuperResolutionMethod::FSR1: return getFSR1ScaleFactor();
			default: CHECK_NO_ENTRY(); break;
		}
		return 1.0f;
	}

}
