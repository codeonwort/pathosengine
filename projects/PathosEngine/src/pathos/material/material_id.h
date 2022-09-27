#pragma once

#include "badger/types/int_types.h"

namespace pathos {

	// #todo-material: Deprecate this
	enum class MATERIAL_ID : uint8 {
		PBR_TEXTURE             = 2,
		TRANSLUCENT_SOLID_COLOR = 3,
		ALPHA_ONLY_TEXTURE      = 4,

		NUM_MATERIAL_IDS = 6,
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
