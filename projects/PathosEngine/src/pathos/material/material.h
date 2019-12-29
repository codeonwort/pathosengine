#pragma once

#include "pathos/named_object.h"
#include "pathos/material/material_id.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/geometry.h"
#include "pathos/light/light.h"

#include "gl_core.h"
#include <vector>

namespace pathos {

	// Base class for all material classes.
	// One material can be applied to multiple meshes.
	class Material : public NamedObject {

	protected:
		// IMPORTANT: Child classes should initialize this in their constructors
		MATERIAL_ID materialID = MATERIAL_ID::INVALID;

	public:
		MATERIAL_ID getMaterialID() { return materialID; }

	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Materials

	// #todo-material: Support emissive and translucent
	// Opaque solid color
	class ColorMaterial : public Material {

	public:
		ColorMaterial();

		void setAlbedo(GLfloat r, GLfloat g, GLfloat b);
		inline void setMetallic(GLfloat inMetallic)   { metallic = inMetallic; }
		inline void setRoughness(GLfloat inRoughness) { roughness = inRoughness; }

		inline void setAlpha(GLfloat a) { alpha = a; }

		inline const GLfloat* getAlbedo() const { return albedo; }
		inline GLfloat getMetallic()      const { return metallic; }
		inline GLfloat getRoughness()     const { return roughness; }

		inline GLfloat getAlpha()         const { return alpha; }

	private:
		GLfloat albedo[3];
		GLfloat metallic;
		GLfloat roughness;
		GLfloat alpha;
		GLuint blendSrcFactor;
		GLuint blendDstFactor;

	};
	
	class TextureMaterial : public Material {

	private:
		GLuint texture;
		GLfloat specular[3];
		bool useLighting = true;
		bool useAlpha = false;

	public:
		TextureMaterial(GLuint texture);

		inline GLuint getTexture() { return texture; }
		inline void setTexture(GLuint _texture) { texture = _texture; }

		void setSpecular(GLfloat r, GLfloat g, GLfloat b);
		inline const GLfloat* getSpecular() const { return specular; }

		inline bool getLighting() { return useLighting; }
		inline void setLighting(bool value) { useLighting = value; } // determine whether this texture will be lit
		inline bool getUseAlpha() { return useAlpha; }
		inline void setUseAlpha(bool value) { useAlpha = value; }

	};
	
	class BumpTextureMaterial : public Material {

	public:
		BumpTextureMaterial(GLuint diffuseTexture, GLuint normalMapTexture);
		inline const GLuint getDiffuseTexture() { return diffuseTexture; }
		inline const GLuint getNormalMapTexture() { return normalMapTexture; }

	protected:
		GLuint diffuseTexture;
		GLuint normalMapTexture;

	};

	class WireframeMaterial : public Material {

	private:
		GLfloat rgba[4];

	public:
		WireframeMaterial(GLfloat, GLfloat, GLfloat, GLfloat = 1.0f);
		inline const GLfloat* getColor() const { return rgba; }
	};
	
	class CubeEnvMapMaterial : public Material {

	protected:
		GLuint texture; // cubemap texture for environmental mapping

	public:
		CubeEnvMapMaterial(GLuint cubeTexture);
		inline GLuint getTexture() const { return texture; }
	};

	class AlphaOnlyTextureMaterial : public Material {

	protected:
		GLuint texture;
		GLfloat color[3];

	public:
		AlphaOnlyTextureMaterial(GLuint texture, GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f);
		inline const GLuint getTexture() { return texture; }
		inline const GLfloat* getColor() const { return color; }
		inline void setColor(GLfloat r, GLfloat g, GLfloat b) { color[0] = r; color[1] = g; color[2] = b; }
	};

	// Cook-Torrance BRDF
	class PBRTextureMaterial : public Material {

	public:
		PBRTextureMaterial(GLuint albedo, GLuint normal, GLuint metallic, GLuint roughness, GLuint ao);
		inline GLuint getAlbedo() const { return tex_albedo; }
		inline GLuint getNormal() const { return tex_normal; }
		inline GLuint getMetallic() const { return tex_metallic; }
		inline GLuint getRoughness() const { return tex_roughness; }
		inline GLuint getAO() const { return tex_ao; }

	protected:
		GLuint tex_albedo;
		GLuint tex_normal;
		GLuint tex_metallic;
		GLuint tex_roughness;
		GLuint tex_ao;

	};

	//////////////////////////////////////////////////////////////////////////
	// #todo-deprecated: Remove these materials

	class ShadowTextureMaterial : public Material {

	protected:
		GLuint texture;

	public:
		ShadowTextureMaterial(GLuint texture);
		inline const GLuint getTexture() { return texture; }
	};

	class ShadowCubeTextureMaterial : public Material {

	protected:
		GLuint texture;
		unsigned int face;
		GLfloat zNear, zFar;

	public:
		ShadowCubeTextureMaterial(GLuint depthCubemapTexture, unsigned int face, GLfloat zNear, GLfloat zFar);
		inline const GLuint getTexture() { return texture; }
		inline const GLuint getFace() { return face; }
		inline const GLfloat getZNear() { return zNear; }
		inline const GLfloat getZFar() { return zFar; }
	};

}
