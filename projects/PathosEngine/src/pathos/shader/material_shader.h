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
		Uint,
		Bool
	};

	template<typename T>
	struct MaterialConstantParameter {
		std::string name;
		EMaterialParameterDataType datatype;
		T value[4];
	};
	struct MaterialTextureParameter {
		std::string name;
		uint32 binding;
		GLuint glTexture;
	};

	class MaterialShader {

	public:
		MaterialShader(const std::string& uniqueName)
			: name(uniqueName)
		{
		}

		// Pack scalar/vector params into a uniform buffer.
		void packUniformBuffer();

		void generateVertexShader();
		void generateFragmentShader();

	private:
		std::string name;

		EMaterialShadingModel shadingModel = EMaterialShadingModel::DEFAULTLIT;

		uint32 uboTotalBytes = 0;
		std::vector<MaterialConstantParameter<float>> floatParameters;
		std::vector<MaterialConstantParameter<int32>> intParameters;
		std::vector<MaterialConstantParameter<uint32>> uintParameters;
		std::vector<MaterialConstantParameter<bool>> boolParameters;
		std::vector<MaterialTextureParameter> textureParameters;

	};

}
