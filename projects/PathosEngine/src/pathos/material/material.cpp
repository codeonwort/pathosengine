#include "pathos/material/material.h"
#include "pathos/shader/material_shader.h"
#include "pathos/shader/material_shader_assembler.h"
#include "pathos/engine.h"

#include "badger/math/minmax.h"

namespace pathos {

	pathos::Material* createMaterialInstance(const char* materialName) {
		const uint32 hash = COMPILE_TIME_CRC32_STR(materialName);
		MaterialShader* ms = MaterialShaderAssembler::get().findMaterialShaderByHash(hash);
		CHECKF(ms != nullptr, "Invalid material name");
		uint32 instanceID = ms->getNextInstanceID();
		Material* material = new Material;
		material->internal_bindMaterialShader(ms, instanceID);
		return material;
	}

	void Material::internal_bindMaterialShader(MaterialShader* inMaterialShader, uint32 inInstanceID) {
		materialShader = inMaterialShader;
		materialInstanceID = inInstanceID;
		materialShader->extractMaterialParameters(constantParameters, textureParameters);
	}

	MaterialConstantParameter* Material::findConstantParameter(const char* name) {
		for (MaterialConstantParameter& mcp : constantParameters) {
			if (mcp.name == name) {
				return &mcp;
			}
		}
		return nullptr;
	}

	void Material::setTextureParameter(const char* name, GLuint glTexture) {
		MaterialTextureParameter* mtp = findTextureParameter(name);
		CHECKF(mtp != nullptr, "Can't find material texture parameter");
		mtp->glTexture = glTexture;
	}

	MaterialTextureParameter* Material::findTextureParameter(const char* name) {
		for (MaterialTextureParameter& mtp : textureParameters) {
			if (mtp.name == name) {
				return &mtp;
			}
		}
		return nullptr;
	}

	void Material::internal_fillUniformBuffer(uint8* uboMemory) {
		for (const MaterialConstantParameter& param : constantParameters) {
			switch (param.datatype) {
			case EMaterialParameterDataType::Float:
			{
				float* ptr = (float*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.fvalue[i];
			}
			break;
			case EMaterialParameterDataType::Int:
			{
				int32* ptr = (int32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.ivalue[i];
			}
			break;
			case EMaterialParameterDataType::Uint:
			{
				uint32* ptr = (uint32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = param.uvalue[i];
			}
			break;
			case EMaterialParameterDataType::Bool:
			{
				uint32* ptr = (uint32*)(uboMemory + param.offset);
				for (uint32 i = 0; i < param.numElements; ++i) ptr[i] = (uint32)param.bvalue[i];
			}
			break;
			default:
				CHECK_NO_ENTRY();
				break;
			}
		}
	}

}

namespace pathos {
	
	////////////////////////////////////////////////////////////////////////////////////
	// ColorMaterial
	ColorMaterial::ColorMaterial() {
		materialID = MATERIAL_ID::SOLID_COLOR;

		setMetallic(0.0f);
		setRoughness(0.9f);
		setAlbedo(0.5f, 0.5f, 0.5f);
		setEmissive(0.0f, 0.0f, 0.0f);

		billboard = false;
		billboardWidth = 10.0f;
	}

	void ColorMaterial::setAlbedo(float r, float g, float b) {
		albedo.x = badger::clamp(0.0f, r, 1.0f);
		albedo.y = badger::clamp(0.0f, g, 1.0f);
		albedo.z = badger::clamp(0.0f, b, 1.0f);
	}

	void ColorMaterial::setMetallic(float inMetallic) {
		metallic = badger::clamp(0.0f, inMetallic, 1.0f);
	}

	void ColorMaterial::setRoughness(float inRoughness) {
		roughness = badger::clamp(0.0f, inRoughness, 1.0f);
	}

	void ColorMaterial::setEmissive(float r, float g, float b) {
		emissive.x = r;
		emissive.y = g;
		emissive.z = b;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// WireframeMaterial
	WireframeMaterial::WireframeMaterial(float r, float g, float b, float a) {
		materialID = MATERIAL_ID::WIREFRAME;
		rgba[0] = r; rgba[1] = g; rgba[2] = b; rgba[3] = a;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// AlphaOnlyTextureMaterial
	AlphaOnlyTextureMaterial::AlphaOnlyTextureMaterial(GLuint texture, float r, float g, float b) :texture(texture) {
		materialID = MATERIAL_ID::ALPHA_ONLY_TEXTURE;
		setColor(r, g, b);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// PBRTextureMaterial
	PBRTextureMaterial* PBRTextureMaterial::createWithFallback(GLuint albedo, GLuint normal /*= 0*/) {
		PBRTextureMaterial* M = new PBRTextureMaterial(albedo, normal,
			gEngine->getSystemTexture2DBlack(),  // metallic
			gEngine->getSystemTexture2DWhite(),  // roughness
			gEngine->getSystemTexture2DWhite()); // localAO
		if (normal == 0) {
			M->setNormal(gEngine->getSystemTexture2DBlue());
		}
		return M;
	}

	PBRTextureMaterial::PBRTextureMaterial(GLuint albedo, GLuint normal, GLuint metallic, GLuint roughness, GLuint ao) {
		materialID = MATERIAL_ID::PBR_TEXTURE;
		tex_albedo = albedo;
		tex_normal = normal;
		tex_metallic = metallic;
		tex_roughness = roughness;
		tex_ao = ao;
	}

}
