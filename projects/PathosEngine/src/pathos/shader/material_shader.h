#pragma once

#include "pathos/material/material_id.h"

#include "badger/types/vector_types.h"
#include <vector>
#include <string>

namespace pathos {

	// #todo-material-assembler: Wanna get rid of GLuint from this header.
	typedef unsigned int GLuint;
	struct MaterialTemplate;
	class ShaderProgram;

	enum class EMaterialParameterDataType : uint32 {
		Float,
		Int,
		Uint,
		Bool
	};

	struct MaterialConstantParameter {
		std::string name;
		EMaterialParameterDataType datatype;
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
		GLuint glTexture = 0;
	};

	class MaterialShader {
		friend class MaterialShaderAssembler;

	public:
		void generateShaderProgram(const std::string& fullpath, const MaterialTemplate* materialTemplate);

	// #todo-material-assembler: private
	public:
		std::string name;
		EMaterialShadingModel shadingModel;
		uint32 uboTotalBytes = 0;
		std::vector<MaterialConstantParameter> constantParameters;
		std::vector<MaterialTextureParameter> textureParameters;

		ShaderProgram* program = nullptr;
		uint32 programHash = 0;

		std::string nameVS;
		std::string nameFS;
	};

}
