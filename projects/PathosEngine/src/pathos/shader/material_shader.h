#pragma once

#include "uniform_buffer.h"
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
		GLuint glTexture = 0;
	};

	class MaterialShader {
		friend class MaterialShaderAssembler;

	public:
		void generateShaderProgram(const std::string& fullpath, const MaterialTemplate* materialTemplate);

		void fillUniformBuffer(uint8* uboMemory);

		// #todo-material-assembler: Template may require less typing?
		void setParameterFloat(const char* name, float value);
		void setParameterVec3(const char* name, const vector3& value);

	// #todo-material-assembler: private
	public:
		std::string name;
		EMaterialShadingModel shadingModel;
		uint32 uboTotalBytes = 0;
		std::vector<MaterialConstantParameter> constantParameters;
		std::vector<MaterialTextureParameter> textureParameters;

		ShaderProgram* program = nullptr;
		uint32 programHash = 0;

		UniformBuffer uboMaterial;
		std::string uboName;
		const uint32 uboBindingPoint = 2; // #todo-material-assembler: UBO binding point

		std::string nameVS;
		std::string nameFS;
	};

}
