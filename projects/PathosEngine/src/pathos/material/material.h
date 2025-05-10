#pragma once

#include "pathos/material/material_id.h"
#include "pathos/material/material_shader.h"
#include "pathos/smart_pointer.h"

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include <vector>

namespace pathos {

	class Texture;
	class MaterialProxy;
	class SceneProxy;

	// a MaterialShader relates to a unique material shader program.
	// a Material is (material shader + own parameters).
	// Multiple materials might share a same material shader, but they can have their own parameters.
	class Material {

	private:
		static Material* createMaterialInstanceRaw(const char* materialName);
	public:
		// Use this to create a Material.
		static assetPtr<Material> createMaterialInstance(const char* materialName);

	private:
		Material() = default;
	public:
		virtual ~Material() = default;

		const std::string& getMaterialName() const { return materialName; }

		template<typename ValueType>
		void setConstantParameter(const char* name, const ValueType& value) {
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
			constexpr size_t valueSize = isBool ? sizeof(ValueType) : (sizeof(ValueType) / 4);
			CHECKF(mcp->numElements == valueSize, "Num elements of MCP and given value are different");

			void* dst = nullptr;
			if constexpr (isFloat) dst = &(mcp->fvalue[0]);
			if constexpr (isInt) dst = &(mcp->ivalue[0]);
			if constexpr (isUint) dst = &(mcp->uvalue[0]);
			if constexpr (isBool) dst = &(mcp->bvalue[0]);
			memcpy_s(dst, sizeof(ValueType), &value, sizeof(ValueType));
		}

		MaterialConstantParameter* findConstantParameter(const char* name);

		void setTextureParameter(const char* name, Texture* texture);

		MaterialTextureParameter* findTextureParameter(const char* name);

		// Only successful when their material shaders are same. Returns true if successful.
		bool copyParametersFrom(Material* other);

	public:
		MaterialShader* internal_getMaterialShader() const { return materialShader; } // #wip: Delete this

		MaterialProxy* createMaterialProxy(SceneProxy* scene) const;

	private:
		void bindMaterialShader(MaterialShader* inMaterialShader, uint32 inInstanceID);

	public:
		bool bWireframe = false;

	private:
		MaterialShader* materialShader = nullptr;
		// Different instances share a same material shader but have different parameters.
		uint32 materialInstanceID = 0xffffffff;
		std::string materialName;

		std::vector<MaterialConstantParameter> constantParameters;
		std::vector<MaterialTextureParameter> textureParameters;
	};

	// Temp util to easily create 'pbr_texture' material.
	assetPtr<Material> createPBRMaterial(Texture* albedoTex, Texture* normalTex = nullptr);

}
