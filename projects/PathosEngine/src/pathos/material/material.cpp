#include "pathos/material/material.h"
#include "pathos/shader/material_shader.h"
#include "pathos/shader/material_shader_assembler.h"
#include "pathos/engine.h"

#include "badger/math/minmax.h"

namespace pathos {

	MaterialShader* findMaterialShader(const char* materialName) {
		return MaterialShaderAssembler::get().findMaterialShader(materialName);
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
