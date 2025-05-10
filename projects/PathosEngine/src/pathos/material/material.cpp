#include "material.h"
#include "pathos/material/material_proxy.h"
#include "pathos/material/material_shader.h"
#include "pathos/material/material_shader_assembler.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/rhi/texture.h"
#include "pathos/util/engine_util.h"

#include "badger/math/minmax.h"

namespace pathos {

	Material* Material::createMaterialInstanceRaw(const char* materialName) {
		const uint32 hash = COMPILE_TIME_CRC32_STR(materialName);
		MaterialShader* ms = MaterialShaderAssembler::get().findMaterialShaderByHash(hash);
		CHECKF(ms != nullptr, "Invalid material name");
		uint32 instanceID = ms->getNextInstanceID();
		Material* material = new Material;
		material->bindMaterialShader(ms, instanceID);
		material->materialName = materialName;
		return material;
	}

	assetPtr<Material> Material::createMaterialInstance(const char* materialName) {
		return assetPtr<Material>(createMaterialInstanceRaw(materialName));
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

	MaterialProxy* Material::createMaterialProxy(SceneProxy* scene) const {
		MaterialProxy* proxy = ALLOC_RENDER_PROXY<MaterialProxy>(scene);

		proxy->materialShader     = materialShader;
		proxy->materialInstanceID = materialInstanceID;
		proxy->bWireframe         = bWireframe;

		proxy->constantParameters = constantParameters;
		proxy->textureParameters  = textureParameters;

		return proxy;
	}

}

namespace pathos {

	assetPtr<Material> createPBRMaterial(Texture* albedoTex, Texture* normalTex /*= nullptr*/) {
		CHECK(albedoTex != nullptr && albedoTex->isCreated()); // At least albedo must be there.
		assetPtr<Material> M = Material::createMaterialInstance("pbr_texture");

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
