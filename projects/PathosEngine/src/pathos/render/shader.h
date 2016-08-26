#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>
using namespace std;

namespace pathos {

	void compileShader(GLuint shaderID, std::string &code);
	GLuint createProgram(std::string& vsCode, std::string& fsCode);

	class VertexShaderCompiler {
	private:
		GLuint positionLocation, uvLocation, normalLocation, tangentLocation, bitangentLocation;
		bool usePosition, useUV, useNormal, useTangent, useBitangent;
		bool transferPosition = false;
		bool useVarying();
		vector<pair<string, string>> uniforms;
		vector<pair<string, string>> outVars;
		string maincode;
	public:
		VertexShaderCompiler();
		string getCode();
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

	class FragmentShaderCompiler {
	private:
		vector<pair<string, string>> uniforms;
		vector<pair<string, string>> inVars;
		vector<pair<string, string>> outVars;
		unsigned int numDirLights, numPointLights;
		string maincode;
		//vector<string> texSamplers;
	public:
		FragmentShaderCompiler();
		string getCode();
		void clear();
		void inVar(const string& type, const string &name);
		void outVar(const string& type, const string &name);
		void uniform(const string& type, const string& name);
		void textureSampler(const string& samplerName);
		void textureSamplerCube(const string& samplerName);
		void textureSamplerShadow(const string& samplerName);
		void textureSamplerCubeShadow(const string& samplerName);
		void mainCode(const string& code);
		void directionalLights(unsigned int num);
		void pointLights(unsigned int num);
	};

}