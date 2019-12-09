#pragma once

// OpenGL
#include "gl_core.h"

// STL
#include <string>
#include <vector>
using namespace std;

namespace pathos {

	// Forward declaration
	class ShaderSource;
	class Shader;

	// Utilities
	GLuint createProgram(const std::string& vsCode, const std::string& fsCode, const char* debugName);     // only vertex and fragment shaders
	GLuint createProgram(Shader& vs, Shader& fs, const char* debugName);                                   // only vertex and fragment shaders
	GLuint createProgram(std::vector<Shader*>& shaders, const char* debugName);                            // multiple shaders
	GLuint createProgram(Shader& shader, const char* debugName);                                           // single shader (for program pipeline)
	GLuint createComputeProgram(const std::string& shader_source, const char* debugName);                  // compute shader

	// #todo: remove this
	GLuint createProgram(std::vector<ShaderSource*>& sources, const char* debugName);                      // multiple shader sources
	// #todo: temp
	void dumpShaderSource(ShaderSource& shader, const char* filename);

	/////////////////////////////////////////////////////////////////////////////////////////////

	struct ShaderVariable {

		string type;
		string name;
		unsigned int arrayLength;

		ShaderVariable(const string& type, const string& name, unsigned int length);

		inline string getDecl() {
			if (arrayLength == 0) return (type + " " + name);
			return (type + " " + name + "[" + to_string(arrayLength) + "]");
		}

	};

	// Takes shader sources and compile them.
	// sources come from ShaderSource (setSource) or physical files (loadSource).
	class Shader {
	public:
		Shader(GLenum type, const char* debugName = "");
		virtual ~Shader();

		void setSource(const char* source);
		inline void setSource(const std::string& source);

		bool loadSource(const char* filepath);
		inline bool loadSource(const std::string& filepath);

		bool compile();

		const GLuint getName() { return glName; }
		const char* getErrorLog() { return errorLog.c_str(); }

	protected:
		GLuint glName;
		GLenum type;
		std::vector<std::string> source;
		std::string errorLog;
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	// #todo: remove this and all children
	// generates shader source codes for Shader::setSource().
	class ShaderSource {
	protected:
		GLenum shaderType; // set default value in child's constructor

	public:
		virtual std::string getCode() = 0;
		GLenum getShaderType() { return shaderType; }
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
		vector<ShaderVariable> uniforms;
		vector<ShaderVariable> outVars;
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

		inline GLuint getPositionLocation() { return positionLocation; }
		inline GLuint getNormalLocation() { return normalLocation; }
		inline GLuint getUVLocation() { return uvLocation; }
		inline GLuint getTangentLocation() { return tangentLocation; }
		inline GLuint getBitangentLocation() { return bitangentLocation; }

		void mainCode(const string& code);
		void outVar(const string& type, const string &name);
		void outVar(const string& type, const string &name, unsigned int length);
		void uniform(const string& type, const string& name);
		void uniform(const string& type, const string& name, unsigned int length);
		void uniformMat4(const string& name);
	};

	class FragmentShaderSource : public ShaderSource {
	private:
		vector<ShaderVariable> uniforms;
		vector<ShaderVariable> inVars;
		vector<ShaderVariable> outVars;
		unsigned int numDirLights, numPointLights;
		string interfaceBlock;
		string maincode;
	public:
		FragmentShaderSource();
		virtual string getCode();
		void clear();
		void inVar(const string& type, const string &name);
		void outVar(const string& type, const string &name);
		void uniform(const string& type, const string& name);
		void inVar(const string& type, const string &name, unsigned int length);
		void uniform(const string& type, const string& name, unsigned int length);
		void textureSampler(const string& samplerName);
		void textureSamplerCube(const string& samplerName);
		void textureSamplerShadow(const string& samplerName);
		void textureSamplerCubeShadow(const string& samplerName);
		void interfaceBlockName(const string& name);
		void mainCode(const string& code);
		void directionalLights(unsigned int num);
		void pointLights(unsigned int num);
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