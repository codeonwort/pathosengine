#pragma once

#include "badger/types/int_types.h"

namespace pathos {

	// #todo-material: Deprecate this
	enum class MATERIAL_ID : uint8 {
		SOLID_COLOR = 0,
		WIREFRAME = 2,
		ALPHA_ONLY_TEXTURE = 7,
		PBR_TEXTURE = 8,
		TRANSLUCENT_SOLID_COLOR = 9,

		NUM_MATERIAL_IDS = 10,
		INVALID = 0xff,
	};

	// #todo-material-assembler: Use this
	enum class EMaterialShadingModel : uint8 {
		UNLIT = 0,
		DEFAULTLIT = 1,
		TRANSLUCENT = 2,
		//SKIN = 3,
		//HAIR = 4,

		NUM_DOMAINS = 3,
	};
	static_assert((uint32)EMaterialShadingModel::NUM_DOMAINS <= 0xff, "Too many shading models");

}
