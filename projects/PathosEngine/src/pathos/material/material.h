#pragma once

#include "pathos/core.h"
#include "pathos/material/material_id.h"
#include "pathos/camera/camera.h"
#include "pathos/render/shader.h"
#include "pathos/mesh/geometry.h"
#include "pathos/light/light.h"
//#include "pathos/render/ReflectionMethod.h"
#include "GL/gl_core_4_3.h"
#include <vector>

namespace pathos {

	// Base class for all material calsses.
	// One material can be applied to multiple meshes.
	class MeshMaterial : public NamedObject {

	protected:
		// IMPORTANT: Child classes should initialize this in their constructors
		MATERIAL_ID materialID = MATERIAL_ID::INVALID;

		//std::vector<DirectionalLight*> directionalLights;
		//std::vector<PointLight*> pointLights;

	public:
		MATERIAL_ID getMaterialID() { return materialID; }

		//PlaneReflection* getReflectionMethod();
		//void setReflectionMethod(PlaneReflection*);
		/*
		void addLight(DirectionalLight* light);
		void addLight(PointLight* light);
		const std::vector<DirectionalLight*>& getDirectionalLights();
		const std::vector<PointLight*>& getPointLights();
		const std::vector<GLfloat>& getDirectionalLightDirectionBuffer() { return dirLightDirections; }
		const std::vector<GLfloat>& getDirectionalLightColorBuffer() { return dirLightColors; }
		const std::vector<GLfloat>& getPointLightPositionBuffer() { return pointLightPositions; }
		const std::vector<GLfloat>& getPointLightColorBuffer() { return pointLightColors; }
		*/
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Materials

	class ColorMaterial : public MeshMaterial {

	private:
		GLfloat ambient[3], diffuse[3], specular[3], alpha;
		GLuint blendSrcFactor, blendDstFactor;

	public:
		ColorMaterial();
		void setAmbient(GLfloat r, GLfloat g, GLfloat b);
		void setDiffuse(GLfloat r, GLfloat g, GLfloat b);
		void setSpecular(GLfloat r, GLfloat g, GLfloat b);
		void setAlpha(GLfloat a);
		void setBlendFactor(GLuint, GLuint);

		inline const GLfloat* getAmbient() const { return ambient; }
		inline const GLfloat* getDiffuse() const { return diffuse; }
		inline const GLfloat* getSpecular() const { return specular; }
		inline GLfloat getAlpha() const { return alpha; }
		inline GLuint getBlendSrcFactor() const { return blendSrcFactor; }
		inline GLuint getBlendDstFactor() const { return blendDstFactor; }
	};
	
	class TextureMaterial : public MeshMaterial {

	private:
		GLuint texture;
		GLfloat specular[3];
		bool useLighting = true;
		bool useAlpha = false;
		//string channelMapping;

	public:
		TextureMaterial(GLuint texture);
		//TextureMaterial(GLuint texture, bool useAlpha = false, string channelMapping = "rgb");

		inline GLuint getTexture() { return texture; }
		inline void setTexture(GLuint _texture) { texture = _texture; }

		void setSpecular(GLfloat r, GLfloat g, GLfloat b);
		inline const GLfloat* getSpecular() const { return specular; }

		inline bool getLighting() { return useLighting; }
		inline void setLighting(bool value) { useLighting = value; } // determine whether this texture will be lit
		inline bool getUseAlpha() { return useAlpha; }
		inline void setUseAlpha(bool value) { useAlpha = value; }

	};
	
	class BumpTextureMaterial : public MeshMaterial {

	protected:
		GLuint diffuseTexture, normalMapTexture;

	public:
		BumpTextureMaterial(GLuint diffuseTexture, GLuint normalMapTexture);
		inline const GLuint getDiffuseTexture() { return diffuseTexture; }
		inline const GLuint getNormalMapTexture() { return normalMapTexture; }
	};
	
	class ShadowTextureMaterial : public MeshMaterial {

	protected:
		GLuint texture;

	public:
		ShadowTextureMaterial(GLuint texture);
		inline const GLuint getTexture() { return texture; }
	};

	class ShadowCubeTextureMaterial : public MeshMaterial {

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

	class WireframeMaterial : public MeshMaterial {

	private:
		GLfloat rgba[4];

	public:
		WireframeMaterial(GLfloat, GLfloat, GLfloat, GLfloat = 1.0f);
		inline const GLfloat* getColor() const { return rgba; }
	};
	
	class CubeEnvMapMaterial : public MeshMaterial {

	protected:
		GLuint texture; // cubemap texture for environmental mapping

	public:
		CubeEnvMapMaterial(GLuint cubeTexture);
		inline const GLuint getTexture() { return texture; }
	};

	class AlphaOnlyTextureMaterial : public MeshMaterial {

	protected:
		GLuint texture;
		GLfloat color[3];

	public:
		AlphaOnlyTextureMaterial(GLuint texture, GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f);
		inline const GLuint getTexture() { return texture; }
		inline const GLfloat* getColor() const { return color; }
		inline void setColor(GLfloat r, GLfloat g, GLfloat b) { color[0] = r; color[1] = g; color[2] = b; }
	};

}