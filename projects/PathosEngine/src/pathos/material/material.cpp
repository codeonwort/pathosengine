#include "pathos/material/material.h"

namespace pathos {
	
	////////////////////////////////////////////////////////////////////////////////////
	// ColorMaterial
	ColorMaterial::ColorMaterial() {
		materialID = MATERIAL_ID::SOLID_COLOR;
		setMetallic(0.5f);
		setRoughness(0.1f);
		setAlbedo(0.5f, 0.5f, 0.5f);
		setEmissive(0.0f, 0.0f, 0.0f);
	}

	void ColorMaterial::setAlbedo(GLfloat r, GLfloat g, GLfloat b) {
		albedo.x = r;
		albedo.y = g;
		albedo.z = b;
	}

	void ColorMaterial::setEmissive(GLfloat r, GLfloat g, GLfloat b) {
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

	void TextureMaterial::setSpecular(GLfloat r, GLfloat g, GLfloat b) { specular[0] = r; specular[1] = g; specular[2] = b; }

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
	WireframeMaterial::WireframeMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
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
	AlphaOnlyTextureMaterial::AlphaOnlyTextureMaterial(GLuint texture, GLfloat r, GLfloat g, GLfloat b) :texture(texture) {
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
