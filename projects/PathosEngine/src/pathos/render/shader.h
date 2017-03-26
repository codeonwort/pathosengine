#pragma once

// OpenGL
#include <GL/glew.h>

// STL
#include <string>
#include <vector>
using namespace std;

namespace pathos {

	// Forward declaration
	class ShaderSource;

	// Utilities
	void compileShader(GLuint shaderID, std::string &code);
	GLuint createProgram(std::string& vsCode, std::string& fsCode);
	GLuint createProgram(std::vector<ShaderSource*>& shaders);
	//GLuint loadShaderFromFile(GLuint shaderType, std::string& filepath);

	/////////////////////////////////////////////////////////////////////////////////////////////

	class ShaderSource {
	protected:
		GLuint shaderType; // set default value in child's constructor
	public:
		virtual string getCode() = 0;
		GLuint getShaderType() { return shaderType; }
	};

	/**
	 * Auto-generated variables and uniforms for vertex shaders:
	 * vs_out {
	 *   vec3 position;		// if usePosition = true
	 *   vec3 normal;		// if useNormal = true
	 *   vec2 uv;			// if useUV = true
	 * }
	 * mat4 modelTransform;
	 * mat4 mvpTransform;
	 */
	class VertexShaderSource : public ShaderSource {
	private:
		GLuint positionLocation, uvLocation, normalLocation, tangentLocation, bitangentLocation;
		bool usePosition, useUV, useNormal, useTangent, useBitangent;
		bool transferPosition = false;
		bool useVarying();
		vector<pair<string, string>> uniforms;
		vector<pair<string, string>> outVars;
		string maincode;
	public:
		VertexShaderSource();
		virtual string getCode();
		void clear();
		void setUseUV(bool);
		void setUseNormal(bool);
		void setUseTangent(bool);
		void setUseBitangent(bool);
		void setTransferPosition(bool);
		void setPositionLocation(GLuint);
		void setUVLocation(GLuint);
		void setNormalLocation(GLuint);

		void mainCode(const string& code);
		void outVar(const string& type, const string &name);
		void uniform(const string& type, const string& name);
		void uniformMat4(const string& name);
	};

	class FragmentShaderSource : public ShaderSource {
	private:
		vector<pair<string, string>> uniforms;
		vector<pair<string, string>> inVars;
		vector<pair<string, string>> outVars;
		unsigned int numDirLights, numPointLights;
		string interfaceBlock;
		string maincode;
		//vector<string> texSamplers;
	public:
		FragmentShaderSource();
		virtual string getCode();
		void clear();
		void inVar(const string& type, const string &name);
		void outVar(const string& type, const string &name);
		void uniform(const string& type, const string& name);
		void textureSampler(const string& samplerName);
		void textureSamplerCube(const string& samplerName);
		void textureSamplerShadow(const string& samplerName);
		void textureSamplerCubeShadow(const string& samplerName);
		void interfaceBlockName(const string& name);
		void mainCode(const string& code);
		void directionalLights(unsigned int num);
		void pointLights(unsigned int num);
	};

	class TessellationControlShaderSource : public ShaderSource {
	public:
		TessellationControlShaderSource();
		virtual string getCode();
	};

	class TessellationEvaluationShaderSource : public ShaderSource {
	public:
		TessellationEvaluationShaderSource();
		virtual string getCode();
	};

	class GeometryShaderSource : public ShaderSource {
	private:
		vector<pair<string, string>> uniforms;
		vector<pair<string, string>> inVars;
		vector<pair<string, string>> outVars;
		string inPrimitive, outPrimitive;
		unsigned int maxVertices;
		string maincode;
	public:
		GeometryShaderSource();
		GeometryShaderSource(string, string, unsigned int);
		void inVar(const string& type, const string &name);
		void outVar(const string& type, const string &name);
		void uniform(const string& type, const string& name);
		void mainCode(const string& code);
		virtual string getCode();
	};

}