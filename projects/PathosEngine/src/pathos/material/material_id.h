#pragma once

#include "badger/types/int_types.h"

namespace pathos {

	// #todo-material-assembler: Deprecate this
	enum class MATERIAL_ID : uint8 {
		TRANSLUCENT_SOLID_COLOR = 3,

		NUM_MATERIAL_IDS = 4,
		INVALID = 0xff,
	};

	// #todo-material-assembler: Use this
	enum class EMaterialShadingModel : uint8 {
		INVALID     = 0,
		UNLIT       = 1,
		DEFAULTLIT  = 2,
		TRANSLUCENT = 3,
		//SKIN = 3,
		//HAIR = 4,

		NUM_MODELS = 3,
	};
	static_assert((uint32)EMaterialShadingModel::NUM_MODELS <= 0xff, "Too many shading models");

}
