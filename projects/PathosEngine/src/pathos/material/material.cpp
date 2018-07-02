#include "pathos/material/material.h"

namespace pathos {

	/*
	PlaneReflection* MeshMaterial::getReflectionMethod() { return reflectionMethod; }
	void MeshMaterial::setReflectionMethod(PlaneReflection* pr) {
		reflectionMethod = pr;
		pr->setReflector(owner);
		programDirty = true;
	}
	*/
	
	////////////////////////////////////////////////////////////////////////////////////
	// ColorMaterial
	ColorMaterial::ColorMaterial() {
		materialID = MATERIAL_ID::SOLID_COLOR;
		setAmbient(0.0f, 0.0f, 0.0f);
		setDiffuse(0.5f, 0.5f, 0.5f);
		setSpecular(1.0f, 1.0f, 1.0f);
		setAlpha(1.0f);
		blendSrcFactor = GL_SRC_ALPHA;
		blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
	}

	void ColorMaterial::setAmbient(GLfloat r, GLfloat g, GLfloat b) { ambient[0] = r; ambient[1] = g; ambient[2] = b; }
	void ColorMaterial::setDiffuse(GLfloat r, GLfloat g, GLfloat b) { diffuse[0] = r; diffuse[1] = g; diffuse[2] = b; }
	void ColorMaterial::setSpecular(GLfloat r, GLfloat g, GLfloat b) { specular[0] = r; specular[1] = g; specular[2] = b; }
	void ColorMaterial::setAlpha(GLfloat a) { alpha = a; }
	void ColorMaterial::setBlendFactor(GLuint srcFactor, GLuint dstFactor) { blendSrcFactor = srcFactor; blendDstFactor = dstFactor; }

	////////////////////////////////////////////////////////////////////////////////////
	// TextureMaterial
	TextureMaterial::TextureMaterial(GLuint diffuseTexture) : texture(diffuseTexture) {
		materialID = MATERIAL_ID::FLAT_TEXTURE;
		setSpecular(1.0f, 1.0f, 1.0f);
	}

	void TextureMaterial::setSpecular(GLfloat r, GLfloat g, GLfloat b) { specular[0] = r; specular[1] = g; specular[2] = b; }

	////////////////////////////////////////////////////////////////////////////////////
	// BumpTextureMaterial
	BumpTextureMaterial::BumpTextureMaterial(GLuint diffuseTexture, GLuint normalMapTexture)
		: diffuseTexture(diffuseTexture), normalMapTexture(normalMapTexture)
	{
		materialID = MATERIAL_ID::BUMP_TEXTURE;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// ShadowTextureMaterial
	ShadowTextureMaterial::ShadowTextureMaterial(GLuint texture) :texture(texture) {
		materialID = MATERIAL_ID::SHADOW_TEXTURE;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// ShadowCubeTextureMaterial
	ShadowCubeTextureMaterial::ShadowCubeTextureMaterial(GLuint texture, unsigned int face, GLfloat zNear, GLfloat zFar)
		: texture(texture), face(face), zNear(zNear), zFar(zFar)
	{
		materialID = MATERIAL_ID::CUBEMAP_SHADOW_TEXTURE;
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