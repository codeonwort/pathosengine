#pragma once

#include "badger/types/int_types.h"
#include <string>

namespace pathos {

	class Texture;

	enum class EMaterialParameterDataType : uint32 {
		Float,
		Int,
		Uint,
		Bool
	};

	struct MaterialConstantParameter {
		std::string name;
		EMaterialParameterDataType datatype;
		uint32 numElements;
		union {
			float fvalue[4];
			int32 ivalue[4];
			uint32 uvalue[4];
			bool bvalue[4];
		};
		uint32 offset; // in UBO
	};

	struct MaterialTextureParameter {
		std::string name;
		uint32 binding;
		Texture* texture = nullptr;
	};

}
