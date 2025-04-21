#include "sky_common.h"
#include "pathos/console.h"

#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_skyLightingUpdateMode("r.skyLightingUpdateMode", 1, "0 = disable, 1 = progressive, 2 = every frame");
	
	ESkyLightingUpdateMode getSkyLightingUpdateMode() {
		int32 value = cvar_skyLightingUpdateMode.getInt();
		value = badger::clamp(0, value, 2);
		return (ESkyLightingUpdateMode)value;
	}

	pathos::ESkyLightingUpdatePhase getNextSkyLightingUpdatePhase(ESkyLightingUpdatePhase current) {
		current = (ESkyLightingUpdatePhase)((uint32)current + 1);
		if (current == ESkyLightingUpdatePhase::MAX) {
			current = (ESkyLightingUpdatePhase)0;
		}
		return current;
	}

}
