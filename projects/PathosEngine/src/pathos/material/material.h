#pragma once

#include "pathos/named_object.h"
#include "pathos/material/material_id.h"
#include "pathos/shader/material_shader.h"

#include "badger/types/vector_types.h"
#include <vector>

// #todo: Don't expose GLuint here
typedef unsigned int GLuint;

namespace pathos {

	MaterialShader* findMaterialShader(const char* materialName);

}

namespace pathos {

	// Base class for all material classes.
	// One material can be applied to multiple meshes.
	class Material : public NamedObject {

	public:
		virtual ~Material() = default;

		MATERIAL_ID getMaterialID() { return materialID; }

		// #todo-material-assembler: Temp initializer.
		// Ultimately, a MaterialShader will be bound when a Material is created.
		void bindMaterialShader(MaterialShader* inMaterialShader);

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
			constexpr size_t valueSize = isBool ? sizeof(ValueType) : (sizeof(ValueType) / 4);
			CHECKF(mcp->numElements == valueSize, "Num elements of MCP and given value are different");

			void* dst = nullptr;
			// #todo-cpp17
			if /*constexpr*/ (isFloat) dst = &(mcp->fvalue[0]);
			if /*constexpr*/ (isInt) dst = &(mcp->ivalue[0]);
			if /*constexpr*/ (isUint) dst = &(mcp->uvalue[0]);
			if /*constexpr*/ (isBool) dst = &(mcp->bvalue[0]);
			memcpy_s(dst, sizeof(ValueType), &value, sizeof(ValueType));
		}

		MaterialConstantParameter* findConstantParameter(const char* name);

		void setTextureParameter(const char* name, GLuint glTexture);

		MaterialTextureParameter* findTextureParameter(const char* name);

	// CAUTION: INTERNAL USE ONLY
	public:
		MaterialShader* internal_getMaterialShader() const { return materialShader; }
		const std::vector<MaterialTextureParameter>& internal_getTextureParameters() const { return textureParameters; }
		void internal_fillUniformBuffer(uint8* uboMemory);

	protected:
		// IMPORTANT: Child classes should initialize this in their constructors
		// #todo-material: Deprecate this.
		MATERIAL_ID materialID = MATERIAL_ID::INVALID;

		// #todo-material-assembler: Migration strategy
		// 1. If 'materialShader' is not null, use it for rendering.
		// 2. Otherwise, use old path.
		// 3. Replace old materials with new materials.
		// 4. Obliterate MATERIAL_ID and Material subclasses.
		MaterialShader* materialShader = nullptr;

		std::vector<MaterialConstantParameter> constantParameters;
		std::vector<MaterialTextureParameter> textureParameters;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Materials

	// Opaque solid color, rendered in gbuffer pass
	class ColorMaterial : public Material {

	public:
		ColorMaterial();

		void setAlbedo(float r, float g, float b);
		void setMetallic(float inMetallic);
		void setRoughness(float inRoughness);
		void setEmissive(float r, float g, float b);

		inline vector3 getAlbedo() const { return albedo; }
		inline float getMetallic() const { return metallic; }
		inline float getRoughness() const { return roughness; }
		inline vector3 getEmissive() const { return emissive; }

	// #todo-material: Hard-coded for now
	public:
		bool billboard;
		float billboardWidth;

	private:
		vector3 albedo;
		float metallic;
		vector3 emissive;
		float roughness;

	};

	// Rendered in translucency pass
	class TranslucentColorMaterial : public Material {
	
	public:
		TranslucentColorMaterial() {
			materialID = MATERIAL_ID::TRANSLUCENT_SOLID_COLOR;
			setAlbedo(1.0f, 1.0f, 1.0f);
			metallic = 0.0f;
			roughness = 0.9f;
			opacity = 0.5f;
		}

		inline vector3 getAlbedo() const        { return albedo; }
		inline float getMetallic() const        { return metallic; }
		inline float getRoughness() const       { return roughness; }
		inline float getOpacity() const         { return opacity; }
		inline vector3 getTransmittance() const { return transmittance; }
	
		inline void setAlbedo(float r, float g, float b)             { albedo.x = r; albedo.y = g; albedo.z = b; }
		inline void setAlbedo(const vector3& rgb)                    { albedo = rgb; }
		inline void setMetallic(float inMetallic)                    { metallic = inMetallic; }
		inline void setRoughness(float inRoughness)                  { roughness = inRoughness; }
		inline void setOpacity(float inOpacity)                      { opacity = inOpacity; }
		inline void setTransmittance(const vector3& inTransmittance) { transmittance = inTransmittance; }
	
	private:
		vector3 albedo;
		float metallic;
		float roughness;
		float opacity;
		vector3 transmittance;
	
	};

	class WireframeMaterial : public Material {

	private:
		float rgba[4];

	public:
		WireframeMaterial(float, float, float, float = 1.0f);
		inline const float* getColor() const { return rgba; }
	};
	
	class AlphaOnlyTextureMaterial : public Material {

	protected:
		GLuint texture;
		float color[3];

	public:
		AlphaOnlyTextureMaterial(GLuint texture, float r = 1.0f, float g = 1.0f, float b = 1.0f);
		inline const GLuint getTexture() { return texture; }
		inline const float* getColor() const { return color; }
		inline void setTexture(GLuint inTexture) { texture = inTexture; }
		inline void setColor(float r, float g, float b) { color[0] = r; color[1] = g; color[2] = b; }
	};

	// Cook-Torrance BRDF
	class PBRTextureMaterial : public Material {

	public:
		static PBRTextureMaterial* createWithFallback(GLuint albedo, GLuint normal = 0);

		PBRTextureMaterial(GLuint albedo, GLuint normal, GLuint metallic, GLuint roughness, GLuint ao);
		inline GLuint getAlbedo()    const { return tex_albedo;    }
		inline GLuint getNormal()    const { return tex_normal;    }
		inline GLuint getMetallic()  const { return tex_metallic;  }
		inline GLuint getRoughness() const { return tex_roughness; }
		inline GLuint getAO()        const { return tex_ao;        }
		inline void setAlbedo(GLuint inTexture)    { tex_albedo = inTexture;    }
		inline void setNormal(GLuint inTexture)    { tex_normal = inTexture;    }
		inline void setMetallic(GLuint inTexture)  { tex_metallic = inTexture;  }
		inline void setRoughness(GLuint inTexture) { tex_roughness = inTexture; }
		inline void setAO(GLuint inTexture)        { tex_ao = inTexture;        }

		// #todo-material: hack
		bool useTriplanarMapping = false;
		bool writeAllPixels = true; // true = opaque, false = masked

	protected:
		GLuint tex_albedo;
		GLuint tex_normal;
		GLuint tex_metallic;
		GLuint tex_roughness;
		GLuint tex_ao;

	};

}
