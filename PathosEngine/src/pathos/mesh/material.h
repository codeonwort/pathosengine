#pragma once

#include <vector>
#include <GL/glew.h>
#include <pathos/core.h>
#include <pathos/mesh/geometry.h>
#include <pathos/mesh/shader.h>
#include <pathos/mesh/shadow.h>
#include <pathos/light/light.h>
//#include <BossEngine/Render/ReflectionMethod.h>

/* CAUTION
- a MaterialPass must be added by addPass(), not directly calling push_back() to std::vector<MaterialPass*> passes
*/

namespace pathos {

	class MeshMaterial;
		class ColorMaterial;
		class TextureMaterial;
	class MeshMaterialPass;
		class ColorMaterialPass;
		class TextureMaterialPass;

	// classes in other headers
	class Mesh;
	class MeshDefaultRenderer;

	/////////////////////////////////////////////////////////////////////////
	// materials

	/**
	* One material can be shared among multiple meshes.<br/>
	* But be aware of that if you apply something like lighting to a material through a mesh
	* then every meshes sharing the material will be affected by it!<br/>
	* For example, if mesh M1, M2, M3 share a material X, then don't add a light to the X through M1, M2, M3. X will be lit 3 times.
	*/
	class MeshMaterial : public NamedObject {
		friend class Mesh;
		friend class MeshDefaultRenderer;
		//friend class SpriteMeshRenderer;
	private:
		Mesh* owner = nullptr;
		MeshGeometry* relatedGeometry;
		glm::mat4 modelMatrix;
		glm::mat4 vpTransform; // view * projection matrix
		glm::vec3 eyeVector; // camera's eye vector

		ShadowMap* shadowMethod = nullptr;
		//PlaneReflection* reflectionMethod = nullptr;

		// these 3 functions will be invoked only by View::render() //
		void activatePass(int index);
		void renderPass(int index);
		void deactivatePass(int index);
		//////////////////////////////////////////////////////////////

		bool programDirty = true;
		void updateProgram();
	protected:
		std::vector<DirectionalLight*> directionalLights;
		std::vector<PointLight*> pointLights;
		std::vector<MeshMaterialPass*> passes;
		std::vector<bool> enabled;
	public:
		MeshMaterial();
		virtual ~MeshMaterial();
		void setGeometry(MeshGeometry* related);

		glm::vec3& getEyeVector();
		glm::mat4& getVPTransform();
		glm::mat4& getModelMatrix();
		void setModelMatrix(const glm::mat4& modelMatrix);
		void setEyeVector(glm::vec3& eye);
		void setVPTransform(glm::mat4&);

		size_t numPasses();
		void addPass(MeshMaterialPass*);
		bool isPassEnabled(int index);
		void enablePass(int index);
		void disablePass(int index);
		
		ShadowMap* getShadowMethod();
		void setShadowMethod(ShadowMap*);

		//PlaneReflection* getReflectionMethod();
		//void setReflectionMethod(PlaneReflection*);

		void addLight(DirectionalLight* light);
		void addLight(PointLight* light);
		const std::vector<DirectionalLight*>& getDirectionalLights();
		const std::vector<PointLight*>& getPointLights();
	};
	
	class ColorMaterial : public MeshMaterial {
	private:
		ColorMaterialPass* pass; // only single pass for ColorMaterial
		void init(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	public:
		ColorMaterial(const glm::vec3& rgb);
		ColorMaterial(const glm::vec4& rgba);
		ColorMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f); // diffuse color and material alpha
		void setAmbientColor(GLfloat r, GLfloat g, GLfloat b);
		void setDiffuseColor(GLfloat r, GLfloat g, GLfloat b);
		void setSpecularColor(GLfloat r, GLfloat g, GLfloat b);
		void setAlpha(GLfloat a);
		void setBlendFactor(GLuint, GLuint);
	};
	
	class TextureMaterial : public MeshMaterial {
	public:
		TextureMaterial(GLuint texID, bool useAlpha = false, string channelMapping = "rgb");
	};

	class ShadowTextureMaterial : public MeshMaterial {
	public:
		ShadowTextureMaterial(GLuint texID);
	};

	class WireframeMaterial : public MeshMaterial {
	private:
		GLfloat red, green, blue, alpha;
	public:
		WireframeMaterial(GLfloat, GLfloat, GLfloat, GLfloat = 1.0f);
	};
	
	//////////////////////////////////////////////////////////////////////////
	// passes
	
	class MeshMaterialPass {
	protected:
		GLuint program;
		MeshGeometry* geometry;
		MeshMaterial* material;
		glm::mat4 modelMatrix;
		VertexShaderCompiler vsCompiler;
		FragmentShaderCompiler fsCompiler;
		void createProgram(std::string &vsCode, std::string &fsCode);
	public:
		MeshMaterialPass();
		virtual ~MeshMaterialPass();
		void setMaterial(MeshMaterial*);
		void setGeometry(MeshGeometry* geom);
		void setModelMatrix(const glm::mat4& modelMatrix);
		void clearCompilers();
		virtual void updateProgram(MeshMaterial*) = 0;
		virtual void activate() = 0;
		virtual void renderMaterial() = 0;
		virtual void deactivate() = 0;
	};
	
	class ColorMaterialPass : public MeshMaterialPass {
	private:
		GLfloat ambient[3], diffuse[3], specular[3], alpha;
		GLuint blendSrcFactor = GL_SRC_ALPHA, blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
	public:
		ColorMaterialPass(GLfloat, GLfloat, GLfloat, GLfloat);
		void setAmbient(GLfloat, GLfloat, GLfloat);
		void setDiffuse(GLfloat, GLfloat, GLfloat);
		void setSpecular(GLfloat, GLfloat, GLfloat);
		void setAlpha(GLfloat);
		void setBlendFactor(GLuint, GLuint);
		virtual void updateProgram(MeshMaterial*);
		virtual void activate();
		virtual void renderMaterial();
		virtual void deactivate();
	};
	
	class TextureMaterialPass : public MeshMaterialPass {
	private:
		GLuint textureID;
		string channelMapping;
		bool useAlpha;
	public:
		TextureMaterialPass(GLuint texID, bool useAlpha = false, string channelMapping = "rgb");
		virtual void updateProgram(MeshMaterial*);
		virtual void activate();
		virtual void renderMaterial();
		virtual void deactivate();
	};

	class ShadowTextureMaterialPass : public MeshMaterialPass {
	private:
		GLuint debugTexture;
	public:
		ShadowTextureMaterialPass(GLuint texID);
		virtual void updateProgram(MeshMaterial*);
		virtual void activate();
		virtual void renderMaterial();
		virtual void deactivate();
	};

	class WireframeMaterialPass : public MeshMaterialPass {
	private:
		GLfloat color[4];
	public:
		WireframeMaterialPass(GLfloat, GLfloat, GLfloat, GLfloat);
		virtual void updateProgram(MeshMaterial*);
		virtual void activate();
		virtual void renderMaterial();
		virtual void deactivate();
	};

}