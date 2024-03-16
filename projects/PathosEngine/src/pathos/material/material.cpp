#include "pathos/material/material.h"
#include "pathos/material/material_shader.h"
#include "pathos/material/material_shader_assembler.h"
#include "pathos/rhi/texture.h"
#include "pathos/engine.h"

#include "badger/math/minmax.h"

namespace pathos {

	Material* Material::createMaterialInstance(const char* materialName) {
		const uint32 hash = COMPILE_TIME_CRC32_STR(materialName);
		MaterialShader* ms = MaterialShaderAssembler::get().findMaterialShaderByHash(hash);
		CHECKF(ms != nullptr, "Invalid material name");
		uint32 instanceID = ms->getNextInstanceID();
		Material* material = new Material;
		material->bindMaterialShader(ms, instanceID);
		material->materialName = materialName;
		return material;
	}

	void Material::bindMaterialShader(MaterialShader* inMaterialShader, uint32 inInstanceID) {
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

	void Material::setTextureParameter(const char* name, Texture* texture) {
		MaterialTextureParameter* mtp = findTextureParameter(name);
		CHECKF(mtp != nullptr, "Can't find material texture parameter");
		mtp->texture = texture;
	}

	MaterialTextureParameter* Material::findTextureParameter(const char* name) {
		for (MaterialTextureParameter& mtp : textureParameters) {
			if (mtp.name == name) {
				return &mtp;
			}
		}
		return nullptr;
	}

	bool Material::copyParametersFrom(Material* other) {
		if (materialShader == nullptr || materialShader != other->materialShader) {
			return false;
		}
		constantParameters = other->constantParameters;
		textureParameters = other->textureParameters;
		return true;
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

	Material* createPBRMaterial(Texture* albedoTex, Texture* normalTex /*= nullptr*/) {
		CHECK(albedoTex != nullptr && albedoTex->isValid()); // At least albedo must be there.
		Material* M = Material::createMaterialInstance("pbr_texture");

		M->setConstantParameter("bOverrideAlbedo", false);
		M->setConstantParameter("bOverrideNormal", false);
		M->setConstantParameter("bOverrideMetallic", false);
		M->setConstantParameter("bOverrideRoughness", false);
		M->setConstantParameter("bOverrideLocalAO", false);
		M->setConstantParameter("emissiveConstant", vector3(0.0f));

		M->setTextureParameter("albedo", albedoTex);
		if (normalTex != 0) {
			M->setTextureParameter("normal", normalTex);
		} else {
			M->setTextureParameter("normal", gEngine->getSystemTexture2DNormalmap());
			//M->setConstantParameter("normalOverride", vector3(0.0f, 0.0f, 1.0f));
			//M->setConstantParameter("bOverrideNormal", true);
		}
		// metallic=0, roughness=1, localAO=1
		M->setTextureParameter("metallic", gEngine->getSystemTexture2DBlack());
		M->setTextureParameter("roughness", gEngine->getSystemTexture2DWhite());
		M->setTextureParameter("localAO", gEngine->getSystemTexture2DWhite());

		return M;
	}

}
