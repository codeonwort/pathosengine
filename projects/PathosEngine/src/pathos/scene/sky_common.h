#pragma once

namespace pathos {

	enum class ESkyLightingUpdatePhase : uint32 {
		RenderFacePosX = 0,
		RenderFaceNegX = 1,
		RenderFacePosY = 2,
		RenderFaceNegY = 3,
		RenderFacePosZ = 4,
		RenderFaceNegZ = 5,
		GenerateMips   = 6,
		DiffuseSH      = 7,
		SpecularFilter = 8,
		MAX            = 9,
	};

	enum class ESkyLightingUpdateMode : uint32 {
		Disabled       = 0,
		Progressive    = 1,
		EveryFrame     = 2,
	};

	ESkyLightingUpdateMode getSkyLightingUpdateMode();

	ESkyLightingUpdatePhase getNextSkyLightingUpdatePhase(ESkyLightingUpdatePhase current);

}
