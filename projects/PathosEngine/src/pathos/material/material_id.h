#pragma once

#include "badger/types/int_types.h"

// each MeshMaterial-derived class should set one of these to its materialID.
// #todo-material: maybe should be uint16 and INVALID be (MAX - 1)?
enum class MATERIAL_ID : uint8 {
	SOLID_COLOR             = 0,
	FLAT_TEXTURE            = 1,
	WIREFRAME               = 2,
	SHADOW_TEXTURE          = 3, // #todo-material: Remove this, because it can be debugged with RenderDoc.
	CUBE_ENV_MAP            = 4,
	BUMP_TEXTURE            = 5,
	CUBEMAP_SHADOW_TEXTURE  = 6, // #todo-material: Remove this, because it can be debugged with RenderDoc.
	ALPHA_ONLY_TEXTURE      = 7,
	PBR_TEXTURE             = 8,
	TRANSLUCENT_SOLID_COLOR = 9,

	NUM_MATERIAL_IDS       = 10,
	INVALID                = 0xff,
};
