#include <pathos/render/shader.h>
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
		std::cout << "start creating a shader program" << std::endl;
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

	GLuint createProgram(std::vector<ShaderSource*>& shaders) {
#ifdef DEBUG
		std::cout << "start creating a shader program" << std::endl;
#endif
		vector<GLuint> shaderIDs;
		for (ShaderSource* it : shaders){
			GLuint id = glCreateShader(it->getShaderType());
			shaderIDs.push_back(id);
			compileShader(id, it->getCode());
		}
#ifdef DEBUG
		std::cout << "linking program" << std::endl;
#endif
		GLuint program = glCreateProgram();
		for (GLuint id : shaderIDs){
			glAttachShader(program, id);
		}
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			cout << "program link error: " << string(infoLog.begin(), infoLog.end()) << endl;
			for (ShaderSource* it : shaders){
				cout << it->getCode() << endl;
			}
		}
#ifdef DEBUG
		std::cout << "link error code: " << glGetError() << std::endl;
#endif
		for (GLuint id : shaderIDs){
			glDeleteShader(id);
		}

		return program;
	}

	bool varComp(pair<string, string>& a, pair<string, string>& b) { return a.second < b.second; }

	/////////////////////////////////////////////////////////////////////////////////////////////

	VertexShaderSource::VertexShaderSource() {
		clear();
		shaderType = GL_VERTEX_SHADER;
	}
	bool VertexShaderSource::useVarying() { return useNormal || useUV; }

	void VertexShaderSource::setUseUV(bool use) { useUV = use; }
	void VertexShaderSource::setUseNormal(bool use) { useNormal = use; }
	void VertexShaderSource::setUseTangent(bool use) { useTangent = use; }
	void VertexShaderSource::setUseBitangent(bool use) { useBitangent = use; }
	void VertexShaderSource::setTransferPosition(bool transfer) { transferPosition = transfer; }
	void VertexShaderSource::setPositionLocation(GLuint loc) { positionLocation = loc; }
	void VertexShaderSource::setUVLocation(GLuint loc) { uvLocation = loc; }
	void VertexShaderSource::setNormalLocation(GLuint loc) { normalLocation = loc; }
	
	void VertexShaderSource::outVar(const string & type, const string & name) {
		for (auto it = outVars.begin(); it != outVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("VertexShaderSource::outVar() - variable already defined with different type");
			}
		}
		outVars.push_back(pair<string, string>(type, name));
	}
	void VertexShaderSource::uniformMat4(const string& name) {
		uniforms.push_back(pair<string, string>("mat4", name));
	}
	void VertexShaderSource::uniform(const string& type, const string& name) {
		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("VertexShaderSource::uniform() - variable already defined with different type");
			}
		}
		uniforms.push_back(pair<string, string>(type, name));
	}
	void VertexShaderSource::mainCode(const string& code) {
		maincode += "  " + code + "\n";
	}

	void VertexShaderSource::clear() {
		usePosition = true;
		useNormal = useUV = useTangent = useBitangent = false;
		positionLocation = 0;
		uvLocation = 1;
		normalLocation = 2;
		tangentLocation = 3;
		bitangentLocation = 4;
		maincode = "";
	}
	string VertexShaderSource::getCode() {
		stringstream src;

		// sort variables by name
		if (useNormal) outVar("vec3", "normal");
		if (useUV) outVar("vec2", "uv");
		if (transferPosition) outVar("vec3", "position");
		std::sort(outVars.begin(), outVars.end(), varComp);

		src << "#version 430 core" << std::endl;
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
			//src << "  vs_out.position = gl_Position.xyz / gl_Position.w;" << std::endl;
			src << "  vs_out.position = vec3(modelTransform * vec4(position, 1));" << std::endl; // position in world space
		src << "}" << std::endl;

		string vcode = src.str();
#ifdef DEBUG
		cout << endl << "vertex shader code" << endl;
		cout << vcode << endl;
#endif
		return vcode;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	FragmentShaderSource::FragmentShaderSource() {
		clear();
		shaderType = GL_FRAGMENT_SHADER;
	}
	void FragmentShaderSource::clear() {
		uniforms.clear();
		inVars.clear();
		outVars.clear();
		numDirLights = numPointLights = 0;
		interfaceBlock = "VS_OUT";
		maincode = "";
	}
	
	string FragmentShaderSource::getCode() {
		stringstream src;
		src << "#version 430 core" << endl;

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
			src << "in " << interfaceBlock << " {\n";
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
	void FragmentShaderSource::textureSampler(const string& samplerName) {
		uniform("sampler2D", samplerName);
		//texSamplers.push_back(samplerName);
	}
	void FragmentShaderSource::textureSamplerCube(const string& samplerName) { uniform("samplerCube", samplerName); }
	void FragmentShaderSource::textureSamplerShadow(const string& samplerName) { uniform("sampler2DShadow", samplerName); }
	void FragmentShaderSource::textureSamplerCubeShadow(const string& samplerName) { uniform("samplerCubeShadow", samplerName); }
	void FragmentShaderSource::uniform(const string & type, const string & name) {
		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("FragmentShaderSource::uniform() - variable already defined with different type");
			}
		}
		uniforms.push_back(pair<string, string>(type, name));
	}
	/**
	* declear an in-variable.
	* if already defined with same name and same type, nothing happens.
	* error thrown if already defined with same name but diffent type.
	*/
	void FragmentShaderSource::inVar(const string & type, const string & name) {
		for (auto it = inVars.begin(); it != inVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("FragmentShaderSource::inVar() - variable already defined with different type");
			}
		}
		inVars.push_back(pair<string, string>(type, name));
	}
	void FragmentShaderSource::outVar(const string & type, const string & name) {
		for (auto it = outVars.begin(); it != outVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("FragmentShaderSource::outVar() - variable already defined with different type");
			}
		}
		outVars.push_back(pair<string, string>(type, name));
	}
	void FragmentShaderSource::interfaceBlockName(const string& name) {
		interfaceBlock = name; // VS_OUT or GS_OUT
	}
	void FragmentShaderSource::mainCode(const string& code) {
		maincode += "  " + code + "\n";
	}
	void FragmentShaderSource::directionalLights(unsigned int num) { numDirLights = num; }
	void FragmentShaderSource::pointLights(unsigned int num) { numPointLights = num; }


	/////////////////////////////////////////////////////////////////////////////////////////////


	TessellationControlShaderSource::TessellationControlShaderSource() {
		shaderType = GL_TESS_CONTROL_SHADER;
	}
	string TessellationControlShaderSource::getCode() {
		throw "not implemeneted";
		return "";
	}


	/////////////////////////////////////////////////////////////////////////////////////////////


	TessellationEvaluationShaderSource::TessellationEvaluationShaderSource() {
		shaderType = GL_TESS_EVALUATION_SHADER;
	}
	string TessellationEvaluationShaderSource::getCode() {
		throw "not implemeneted";
		return "";
	}


	/////////////////////////////////////////////////////////////////////////////////////////////


	GeometryShaderSource::GeometryShaderSource() {
		GeometryShaderSource("triangles", "triangle_strip", 3);
	}
	GeometryShaderSource::GeometryShaderSource(string inPrim, string outPrim, unsigned int maxV){
		shaderType = GL_GEOMETRY_SHADER;
		inPrimitive = inPrim; outPrimitive = outPrim; maxVertices = maxV;
	}

	void GeometryShaderSource::uniform(const string& type, const string& name) {
		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("GeometryShaderSource::uniform() - variable already defined with different type");
			}
		}
		uniforms.push_back(pair<string, string>(type, name));
	}
	void GeometryShaderSource::inVar(const string& type, const string& name) {
		for (auto it = inVars.begin(); it != inVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("GeometryShaderSource::inVar() - variable already defined with different type");
			}
		}
		inVars.push_back(pair<string, string>(type, name));
	}
	void GeometryShaderSource::outVar(const string& type, const string& name) {
		for (auto it = outVars.begin(); it != outVars.end(); it++) {
			if ((*it).second == name) {
				if ((*it).first == type) return;
				else throw ("GeometryShaderSource::outVar() - variable already defined with different type");
			}
		}
		outVars.push_back(pair<string, string>(type, name));
	}

	void GeometryShaderSource::mainCode(const string& code) {
		maincode += "  " + code + "\n";
	}

	string GeometryShaderSource::getCode() {
		stringstream src;
		src << "#version 430 core" << endl;

		src << "layout (" << inPrimitive << ") in;" << endl;
		src << "layout (" << outPrimitive << ", max_vertices = " << maxVertices << ") out;" << endl;

		// sort variables by name
		std::sort(inVars.begin(), inVars.end(), varComp);

		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			pair<string, string> &uniform = *it;
			src << "uniform " << uniform.first << " " << uniform.second << ";\n";
		}
		/*if (numDirLights > 0) {
			src << "uniform vec3 dirLightDirs[" << numDirLights << "];\n";
			src << "uniform vec3 dirLightColors[" << numDirLights << "];\n";
		}
		if (numPointLights > 0){
			src << "uniform vec3 pointLightPos[" << numPointLights << "];\n";
			src << "uniform vec3 pointLightColors[" << numPointLights << "];\n";
		}*/
		if (inVars.size() > 0) {
			src << "in VS_OUT {\n";
			for (auto it = inVars.begin(); it != inVars.end(); it++) {
				pair<string, string> &inVar = *it;
				src << "  " << inVar.first << " " << inVar.second << ";\n";
			}
			src << "} gs_in[];\n";
		}
		if (outVars.size() > 0) {
			src << "out GS_OUT {\n";
			for (auto it = outVars.begin(); it != outVars.end(); it++) {
				pair<string, string> &outVar = *it;
				src << "  " << outVar.first << " " << outVar.second << ";\n";
			}
			src << "} gs_out;\n";
		}
		src << "void main() {\n";
		src << maincode;
		src << "}\n";

		string gcode = src.str();
#ifdef DEBUG
		cout << endl << "geometry shader code" << endl;
		cout << fcode << endl;
#endif
		return gcode;
	}

}