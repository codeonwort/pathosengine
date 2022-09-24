#pragma once

#include "pathos/material/material_id.h"

#include "badger/types/vector_types.h"
#include <vector>
#include <string>

namespace pathos {

	// #todo-material-assembler: Wanna get rid of GLuint from this header.
	typedef unsigned int GLuint;

	enum class EMaterialParameterDataType : uint32 {
		Float,
		Int,
		Uint
	};

	struct MaterialScalarParameter {
		std::string name;
		EMaterialParameterDataType datatype;
		float value;
	};

	struct MaterialVectorParameter {
		std::string name;
		EMaterialParameterDataType datatype;
		vector4 value;
	};

	struct MaterialTextureParameter {
		std::string name;
		uint32 binding;
		GLuint glTexture;
	};

	class MaterialShader {

	public:
		// Pack scalar/vector params into a uniform buffer.
		void packUniformBuffer();

		void generateVertexShader();
		void generateFragmentShader();

	private:
		EMaterialShadingModel shadingModel = EMaterialShadingModel::DEFAULTLIT;

		std::vector<MaterialScalarParameter> scalarParameters;
		std::vector<MaterialVectorParameter> vectorParameters;
		std::vector<MaterialTextureParameter> textureParameters;

	};

}
