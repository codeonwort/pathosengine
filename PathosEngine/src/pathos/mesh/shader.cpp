#include <pathos/mesh/shader.h>
#include <sstream>
#include <iostream>
#include <algorithm>

//#define DEBUG
using namespace std;

namespace pathos {

	void compileShader(GLuint id, std::string &code) {
#ifdef DEBUG
		std::cout << "compiling a shader" << std::endl;
#endif
		char const* ptr = code.c_str();
		glShaderSource(id, 1, &ptr, NULL);
		glCompileShader(id);

		// check shader compile error
		GLint success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE) {
			GLint logSize;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logSize);
			GLchar* log = new GLchar[logSize];
			glGetShaderInfoLog(id, logSize, NULL, log);
			std::cout << "shader compile error: " << log;
			std::cout << code << endl;
			delete[] log;
		}
	}
	GLuint createProgram(std::string& vsCode, std::string& fsCode) {
#ifdef DEBUG
		std::cout << "start create program" << std::endl;
#endif
		GLuint vsid = glCreateShader(GL_VERTEX_SHADER);
		GLuint fsid = glCreateShader(GL_FRAGMENT_SHADER);
		compileShader(vsid, vsCode);
		compileShader(fsid, fsCode);
#ifdef DEBUG
		std::cout << "linking program" << std::endl;
#endif
		GLuint program = glCreateProgram();
		glAttachShader(program, vsid);
		glAttachShader(program, fsid);
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			cout << "program link error: " << string(infoLog.begin(), infoLog.end()) << endl;
			cout << "<vertex shader> " << vsCode << endl;
			cout << "<frag shader> " << fsCode << endl;
		}
#ifdef DEBUG
		std::cout << "link error code: " << glGetError() << std::endl;
#endif
		glDeleteShader(vsid);
		glDeleteShader(fsid);

		return program;
	}

	bool varComp(pair<string, string>& a, pair<string, string>& b) { return a.second < b.second; }

	/////////////////////////////////////////////////////////////////////////////////////////////

	VertexShaderCompiler::VertexShaderCompiler() { clear(); }
	bool VertexShaderCompiler::useVarying() { return useNormal || useUV; }

	void VertexShaderCompiler::setUseUV(bool use) { useUV = use; }
	void VertexShaderCompiler::setUseNormal(bool use) { useNormal = use; }
	void VertexShaderCompiler::setUseTangent(bool use) { useTangent = use; }
	void VertexShaderCompiler::setUseBitangent(bool use) { useBitangent = use; }
	void VertexShaderCompiler::setTransferPosition(bool transfer) { transferPosition = transfer; }
	void VertexShaderCompiler::setPositionLocation(GLuint loc) { positionLocation = loc; }
	void VertexShaderCompiler::setUVLocation(GLuint loc) { uvLocation = loc; }
	void VertexShaderCompiler::setNormalLocation(GLuint loc) { normalLocation = loc; }
	
	void VertexShaderCompiler::outVar(const string & type, const string & name) {
		for (auto it = outVars.begin(); it != outVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("VertexShaderCompiler::outVar() - variable already defined with different type");
			}
		}
		outVars.push_back(pair<string, string>(type, name));
	}
	void VertexShaderCompiler::uniformMat4(const string& name) {
		uniforms.push_back(pair<string, string>("mat4", name));
	}
	void VertexShaderCompiler::uniform(const string& type, const string& name) {
		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("VertexShaderCompiler::uniform() - variable already defined with different type");
			}
		}
		uniforms.push_back(pair<string, string>(type, name));
	}
	void VertexShaderCompiler::mainCode(const string& code) {
		maincode += "  " + code + "\n";
	}

	void VertexShaderCompiler::clear() {
		usePosition = true;
		useNormal = useUV = useTangent = useBitangent = false;
		positionLocation = 0;
		uvLocation = 1;
		normalLocation = 2;
		tangentLocation = 3;
		bitangentLocation = 4;
		maincode = "";
	}
	string VertexShaderCompiler::getCode() {
		stringstream src;

		// sort variables by name
		if (useNormal) outVar("vec3", "normal");
		if (useUV) outVar("vec2", "uv");
		if (transferPosition) outVar("vec3", "position");
		std::sort(outVars.begin(), outVars.end(), varComp);

		src << "#version 330 core" << std::endl;
		if (usePosition)
			src << "layout (location = " << positionLocation << ") in vec3 position;" << std::endl;
		if (useNormal)
			src << "layout (location = " << normalLocation << ") in vec3 normal;" << std::endl;
		if (useUV)
			src << "layout (location = " << uvLocation << ") in vec2 uv;" << std::endl;
		if (useTangent)
			src << "layout (location = " << tangentLocation << ") in vec3 tangent;" << std::endl;
		if (useBitangent)
			src << "layout (location = " << bitangentLocation << ") in vec3 bitangent;" << std::endl;
		if (useVarying() || outVars.size() > 0) {
			src << "out VS_OUT {" << std::endl;
			for (auto it = outVars.begin(); it != outVars.end(); it++) {
				src << "  " << it->first << " " << it->second << ";\n";
			}
			src << "} vs_out;" << std::endl;
		}
		src << "uniform mat4 modelTransform;" << std::endl;
		src << "uniform mat4 mvpTransform;" << std::endl;
		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			pair<string, string> &uniform = *it;
			src << "uniform " << uniform.first << " " << uniform.second << ";\n";
		}
		//src << "out gl_PerVertex{ vec4 gl_Position; };" << std::endl;
		src << "void main() {" << std::endl;
		src << maincode;
		if (useNormal)
			src << "  vs_out.normal = normalize((modelTransform * vec4(normal, 0.0)).xyz);" << std::endl;
		if (useUV)
			src << "  vs_out.uv = uv;" << std::endl;
		src << "  gl_Position = mvpTransform * vec4(position, 1.0);" << std::endl;
		if (transferPosition)
			src << "  vs_out.position = gl_Position.xyz / gl_Position.w;" << std::endl;
		src << "}" << std::endl;

		string vcode = src.str();
#ifdef DEBUG
		cout << endl << "vertex shader code" << endl;
		cout << vcode << endl;
#endif
		return vcode;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	FragmentShaderCompiler::FragmentShaderCompiler() {
		clear();
	}
	void FragmentShaderCompiler::clear() {
		uniforms.clear();
		inVars.clear();
		outVars.clear();
		numDirLights = numPointLights = 0;
		maincode = "";
	}
	
	string FragmentShaderCompiler::getCode() {
		stringstream src;
		src << "#version 330 core" << endl;

		// sort variables by name
		std::sort(inVars.begin(), inVars.end(), varComp);

		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			pair<string, string> &uniform = *it;
			src << "uniform " << uniform.first << " " << uniform.second << ";\n";
		}
		if (numDirLights > 0) {
			src << "uniform vec3 dirLightDirs[" << numDirLights << "];\n";
			src << "uniform vec3 dirLightColors[" << numDirLights << "];\n";
		}
		if (numPointLights > 0){
			src << "uniform vec3 pointLightPos[" << numPointLights << "];\n";
			src << "uniform vec3 pointLightColors[" << numPointLights << "];\n";
		}
		if (inVars.size() > 0) {
			src << "in VS_OUT {\n";
			for (auto it = inVars.begin(); it != inVars.end(); it++) {
				pair<string, string> &inVar = *it;
				src << "  " << inVar.first << " " << inVar.second << ";\n";
			}
			src << "} fs_in;\n";
		}
		for (auto it = outVars.begin(); it != outVars.end(); it++) {
			pair<string, string> &outVar = *it;
			src << "out " << outVar.first << " " << outVar.second << ";\n";
		}
		src << "void main() {\n";
		src << maincode;
		src << "}\n";

		string fcode = src.str();
#ifdef DEBUG
		cout << endl << "fragment shader code" << endl;
		cout << fcode << endl;
#endif
		return fcode;
	}
	void FragmentShaderCompiler::textureSampler(const string& samplerName) {
		uniform("sampler2D", samplerName);
		//texSamplers.push_back(samplerName);
	}
	void FragmentShaderCompiler::textureSamplerCube(const string& samplerName) { uniform("samplerCube", samplerName); }
	void FragmentShaderCompiler::textureSamplerShadow(const string& samplerName) { uniform("sampler2DShadow", samplerName); }
	void FragmentShaderCompiler::textureSamplerCubeShadow(const string& samplerName) { uniform("samplerCubeShadow", samplerName); }
	void FragmentShaderCompiler::uniform(const string & type, const string & name) {
		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("FragmentShaderCompiler::uniform() - variable already defined with different type");
			}
		}
		uniforms.push_back(pair<string, string>(type, name));
	}
	/**
	* declear an in-variable.
	* if already defined with same name and same type, nothing happens.
	* error thrown if already defined with same name but diffent type.
	*/
	void FragmentShaderCompiler::inVar(const string & type, const string & name) {
		for (auto it = inVars.begin(); it != inVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("FragmentShaderCompiler::inVar() - variable already defined with different type");
			}
		}
		inVars.push_back(pair<string, string>(type, name));
	}
	void FragmentShaderCompiler::outVar(const string & type, const string & name) {
		for (auto it = outVars.begin(); it != outVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("FragmentShaderCompiler::outVar() - variable already defined with different type");
			}
		}
		outVars.push_back(pair<string, string>(type, name));
	}
	void FragmentShaderCompiler::mainCode(const string& code) {
		maincode += "  " + code + "\n";
	}
	void FragmentShaderCompiler::directionalLights(unsigned int num) { numDirLights = num; }
	void FragmentShaderCompiler::pointLights(unsigned int num) { numPointLights = num; }

}