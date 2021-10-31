#pragma once

#include "badger/types/vector_types.h"

#include "pathos/named_object.h"
#include "pathos/material/material_id.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/mesh/geometry.h"

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
		virtual ~Material() = default;

		MATERIAL_ID getMaterialID() { return materialID; }

	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Materials

	// #todo-material: Support translucent
	// Opaque solid color, rendered in gbuffer pass
	class ColorMaterial : public Material {

	public:
		ColorMaterial();

		void setAlbedo(GLfloat r, GLfloat g, GLfloat b);
		inline void setMetallic(GLfloat inMetallic)   { metallic = inMetallic; }
		inline void setRoughness(GLfloat inRoughness) { roughness = inRoughness; }
		void setEmissive(GLfloat r, GLfloat g, GLfloat b);

		inline const vector3 getAlbedo()   const { return albedo; }
		inline GLfloat getMetallic()       const { return metallic; }
		inline GLfloat getRoughness()      const { return roughness; }
		inline const vector3 getEmissive() const { return emissive; }

	// #todo-material: Hard-coded for now
	public:
		bool billboard;
		float billboardWidth;

	private:
		vector3 albedo;
		GLfloat metallic;
		vector3 emissive;
		GLfloat roughness;

	};

	// Rendered in translucency pass
	class TranslucentColorMaterial : public Material {
	
	public:
		TranslucentColorMaterial() {
			materialID = MATERIAL_ID::TRANSLUCENT_SOLID_COLOR;
			setAlbedo(1.0f, 1.0f, 1.0f);
			metallic = 0.5f;
			roughness = 0.1f;
			opacity = 0.5f;
		}

		inline glm::vec3 getAlbedo() const        { return albedo; }
		inline float getMetallic() const          { return metallic; }
		inline float getRoughness() const         { return roughness; }
		inline float getOpacity() const           { return opacity; }
		inline glm::vec3 getTransmittance() const { return transmittance; }
	
		inline void setAlbedo(GLfloat r, GLfloat g, GLfloat b)         { albedo.x = r; albedo.y = g; albedo.z = b; }
		inline void setAlbedo(const glm::vec3& rgb)                    { albedo = rgb; }
		inline void setMetallic(GLfloat inMetallic)                    { metallic = inMetallic; }
		inline void setRoughness(GLfloat inRoughness)                  { roughness = inRoughness; }
		inline void setOpacity(GLfloat inOpacity)                      { opacity = inOpacity; }
		inline void setTransmittance(const glm::vec3& inTransmittance) { transmittance = inTransmittance; }
	
	private:
		glm::vec3 albedo;
		GLfloat metallic;
		GLfloat roughness;
		GLfloat opacity;
		glm::vec3 transmittance;
	
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
		inline void setTexture(GLuint inTexture) { texture = inTexture; }
		inline void setColor(GLfloat r, GLfloat g, GLfloat b) { color[0] = r; color[1] = g; color[2] = b; }
	};

	// Cook-Torrance BRDF
	class PBRTextureMaterial : public Material {

	public:
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

	protected:
		GLuint tex_albedo;
		GLuint tex_normal;
		GLuint tex_metallic;
		GLuint tex_roughness;
		GLuint tex_ao;

	};

}
