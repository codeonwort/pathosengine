#pragma once

// each MeshMaterial-derived class should set one of these to its materialID.
enum class MATERIAL_ID : int {
	INVALID = -1,
	SOLID_COLOR = 0,
	FLAT_TEXTURE = 1,
	WIREFRAME = 2,
	SHADOW_TEXTURE = 3,
	CUBE_ENV_MAP = 4,
	BUMP_TEXTURE = 5,
	CUBEMAP_SHADOW_TEXTURE = 6,
	ALPHA_ONLY_TEXTURE = 7,
	NUM_MATERIAL_IDS = 8
};