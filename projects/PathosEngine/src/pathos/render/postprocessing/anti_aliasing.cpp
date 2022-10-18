#include "anti_aliasing.h"
#include "pathos/console.h"
#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_anti_aliasing(
		"r.antialiasing.method",
		2,
		"0 = None, 1 = FXAA, 2 = TAA");

	static ConsoleVariable<float> cvar_temporalJitterMultiplier(
		"r.taa.jitterMultiplier",
		1.0f,
		"Temporal jitter multiplier");

	EAntiAliasingMethod getAntiAliasingMethod() {
		int32 total = (int32)EAntiAliasingMethod::NumMethods;
		return (EAntiAliasingMethod)badger::clamp(0, cvar_anti_aliasing.getInt(), total);
	}

	float getTemporalJitterMultiplier() {
		return badger::clamp(0.0f, cvar_temporalJitterMultiplier.getFloat(), 10.0f);
	}

}
