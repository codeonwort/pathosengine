#pragma once

#include "badger/types/int_types.h"

// each MeshMaterial-derived class should set one of these to its materialID.
// #todo-material: maybe should be uint16 and INVALID be (MAX - 1)?
enum class MATERIAL_ID : uint8 {
	SOLID_COLOR             = 0,
	FLAT_TEXTURE            = 1,
	WIREFRAME               = 2,
	CUBE_ENV_MAP            = 4,
	BUMP_TEXTURE            = 5,
	ALPHA_ONLY_TEXTURE      = 7,
	PBR_TEXTURE             = 8,
	TRANSLUCENT_SOLID_COLOR = 9,

	NUM_MATERIAL_IDS       = 10,
	INVALID                = 0xff,
};
