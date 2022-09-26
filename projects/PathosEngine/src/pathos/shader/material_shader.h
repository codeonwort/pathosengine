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

		template<typename ValueType>
		void setConstantParameter(const char* name, const ValueType& value) {
			// #todo-material-assembler: Ugh...
			constexpr bool isFloat = std::is_same<ValueType, float>::value || std::is_same<ValueType, vector2>::value || std::is_same<ValueType, vector3>::value || std::is_same<ValueType, vector4>::value;
			constexpr bool isInt = std::is_same<ValueType, int32>::value || std::is_same<ValueType, vector2i>::value || std::is_same<ValueType, vector3i>::value || std::is_same<ValueType, vector4i>::value;
			constexpr bool isUint = std::is_same<ValueType, uint32>::value || std::is_same<ValueType, vector2ui>::value || std::is_same<ValueType, vector3ui>::value || std::is_same<ValueType, vector4ui>::value;
			constexpr bool isBool = std::is_same<ValueType, bool>::value || std::is_same<ValueType, vector2b>::value || std::is_same<ValueType, vector3b>::value || std::is_same<ValueType, vector4b>::value;
			static_assert(isFloat || isInt || isUint || isBool, "MCP value type is invalid");

			MaterialConstantParameter* mcp = findConstantParameter(name);
			CHECKF(mcp != nullptr, "MCP not found");
			CHECKF(
				(mcp->datatype == EMaterialParameterDataType::Float && isFloat)
				|| (mcp->datatype == EMaterialParameterDataType::Int && isInt)
				|| (mcp->datatype == EMaterialParameterDataType::Uint && isUint)
				|| (mcp->datatype == EMaterialParameterDataType::Bool && isBool), "Element types of MCP and given value are different");
			// #todo-material-assembler: Is it OK for bool vectors?
			CHECKF(mcp->numElements == sizeof(ValueType) / 4, "Num elements of MCP and given value are different");

			void* dst = nullptr;
			// #todo-cpp17
			if /*constexpr*/ (isFloat) dst = &(mcp->fvalue[0]);
			if /*constexpr*/ (isInt) dst = &(mcp->ivalue[0]);
			if /*constexpr*/ (isUint) dst = &(mcp->uvalue[0]);
			if /*constexpr*/ (isBool) dst = &(mcp->bvalue[0]);
			// #todo-material-assembler: Is it OK for bool vectors?
			memcpy_s(dst, sizeof(ValueType), &value, sizeof(ValueType));
		}

		MaterialConstantParameter* findConstantParameter(const char* name) {
			for (MaterialConstantParameter& mcp : constantParameters) {
				if (mcp.name == name) {
					return &mcp;
				}
			}
			return nullptr;
		}

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
