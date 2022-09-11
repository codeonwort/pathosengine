#include "pathos/material/material.h"
#include "badger/math/minmax.h"

namespace pathos {
	
	////////////////////////////////////////////////////////////////////////////////////
	// ColorMaterial
	ColorMaterial::ColorMaterial() {
		materialID = MATERIAL_ID::SOLID_COLOR;

		setMetallic(0.5f);
		setRoughness(0.1f);
		setAlbedo(0.5f, 0.5f, 0.5f);
		setEmissive(0.0f, 0.0f, 0.0f);

		billboard = false;
		billboardWidth = 10.0f;
	}

	void ColorMaterial::setAlbedo(float r, float g, float b) {
		albedo.x = badger::clamp(0.0f, r, 1.0f);
		albedo.y = badger::clamp(0.0f, r, 1.0f);
		albedo.z = badger::clamp(0.0f, r, 1.0f);
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
	// TextureMaterial
	TextureMaterial::TextureMaterial(GLuint diffuseTexture) : texture(diffuseTexture) {
		materialID = MATERIAL_ID::FLAT_TEXTURE;
		setSpecular(1.0f, 1.0f, 1.0f);
	}

	void TextureMaterial::setSpecular(float r, float g, float b) { specular[0] = r; specular[1] = g; specular[2] = b; }

	////////////////////////////////////////////////////////////////////////////////////
	// BumpTextureMaterial
	BumpTextureMaterial::BumpTextureMaterial(GLuint diffuse, GLuint normalMap)
		: diffuseTexture(diffuse)
		, normalMapTexture(normalMap)
	{
		materialID = MATERIAL_ID::BUMP_TEXTURE;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// WireframeMaterial
	WireframeMaterial::WireframeMaterial(float r, float g, float b, float a) {
		materialID = MATERIAL_ID::WIREFRAME;
		rgba[0] = r; rgba[1] = g; rgba[2] = b; rgba[3] = a;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// CubeEnvMapMaterial
	CubeEnvMapMaterial::CubeEnvMapMaterial(GLuint cubeTexture) :texture(cubeTexture) {
		materialID = MATERIAL_ID::CUBE_ENV_MAP;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// AlphaOnlyTextureMaterial
	AlphaOnlyTextureMaterial::AlphaOnlyTextureMaterial(GLuint texture, float r, float g, float b) :texture(texture) {
		materialID = MATERIAL_ID::ALPHA_ONLY_TEXTURE;
		setColor(r, g, b);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// PBRTextureMaterial
	PBRTextureMaterial::PBRTextureMaterial(GLuint albedo, GLuint normal, GLuint metallic, GLuint roughness, GLuint ao) {
		materialID = MATERIAL_ID::PBR_TEXTURE;
		tex_albedo = albedo;
		tex_normal = normal;
		tex_metallic = metallic;
		tex_roughness = roughness;
		tex_ao = ao;
	}

}
