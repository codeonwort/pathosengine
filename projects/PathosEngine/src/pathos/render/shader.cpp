// Pathos
#include "pathos/render/shader.h"

// STL
#include <tuple>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

//#define DEBUG_SHADER

using namespace std;

namespace pathos {

	GLuint createProgram(std::string& vsCode, std::string& fsCode) {
		Shader* vshader = new Shader(GL_VERTEX_SHADER);
		Shader* fshader = new Shader(GL_FRAGMENT_SHADER);
		vshader->setSource(vsCode);
		fshader->setSource(fsCode);
		std::vector<Shader*> shaders = { vshader, fshader };

		GLuint program = createProgram(shaders);
		delete vshader;
		delete fshader;

		return program;
	}

	GLuint createProgram(std::vector<ShaderSource*>& sources) {
		vector<Shader*> shaders(sources.size(), nullptr);
		for (size_t i = 0; i < sources.size(); ++i) {
			ShaderSource* it = sources[i];
			Shader* shader = shaders[i] = new Shader(it->getShaderType());
			shader->setSource(it->getCode());
		}

		GLuint program = createProgram(shaders);
		for (Shader* shader : shaders) delete shader;

		return program;
	}

	// CAUTION: This function does not deallocate Shader objects. Delete them yourself!
	GLuint createProgram(std::vector<Shader*>& shaders) {
#if defined(_DEBUG) && defined(DEBUG_SHADER)
		std::cout << std::endl << "=== start creating a shader program ===" << std::endl;
#endif
		for (Shader* shader : shaders) {
			bool compiled = shader->compile();
			if (!compiled) {
#if defined(_DEBUG)
				std::cerr << "> shader compile error: " << shader->getErrorLog() << endl;
#endif
				break;
			}
		}
#if defined(_DEBUG) && defined(DEBUG_SHADER)
		std::cout << "> linking a program" << std::endl;
#endif
		GLuint program = glCreateProgram();
		for (Shader* shader : shaders) glAttachShader(program, shader->getName());
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			if (maxLength == 0) maxLength = 1000;
			vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
#ifdef _DEBUG
			std::cerr << "> program link error: " << string(infoLog.begin(), infoLog.end()) << endl;
			//for (ShaderSource* it : shaders) cout << it->getCode() << endl;
			std::cerr << "> link error code: " << glGetError() << std::endl;
			std::cerr << "> program was not created. return NULL..." << std::endl;
#endif
			glDeleteProgram(program);
			return 0;
		}
#if defined(_DEBUG) && defined(DEBUG_SHADER)
		std::cout << "=== finish program creation ===" << std::endl << std::endl;
#endif
		return program;
	}

	GLuint createComputeProgram(const std::string& shader_source) {
		Shader* cs = new Shader(GL_COMPUTE_SHADER);
		cs->setSource(shader_source);
		std::vector<Shader*> shaders = { cs };

		GLuint program = createProgram(shaders);
		delete cs;

		return program;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	static bool svarComp(const ShaderVariable& a, const ShaderVariable& b) { return a.name < b.name; }
	static bool varComp(const pair<string, string>& a, const pair<string, string>& b) { return a.second < b.second; }

	static enum class COLLISION { NO_COLLISION, ALREADY_EXIST, COLLIDE };
	static COLLISION collide(vector<ShaderVariable>& vars, const ShaderVariable& var) {
		for (auto it = vars.begin(); it != vars.end(); ++it) {
			if ((*it).name == var.name) {
				if ((*it).type != var.type || (*it).arrayLength != var.arrayLength) {
					return COLLISION::COLLIDE;
				}
				return COLLISION::ALREADY_EXIST;
			}
		}
		return COLLISION::NO_COLLISION;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	ShaderVariable::ShaderVariable(const string& type, const string& name, unsigned int length)
		:type(type), name(name), arrayLength(length) {}

	/////////////////////////////////////////////////////////////////////////////////////////////

	Shader::Shader(GLenum type) {
		name = glCreateShader(type);
		this->type = type;
	}

	Shader::~Shader() {
		glDeleteShader(name);
	}

	void Shader::setSource(const std::string& source) { this->source = source; }
	void Shader::setSource(const char* source) { this->source = source; }

	bool Shader::loadSource(const std::string& filepath) { return loadSource(filepath.c_str()); }
	bool Shader::loadSource(const char* filepath) {
		std::ifstream file(filepath);
		if (!file.is_open()) return false;

		std::ostringstream code;
		code << file.rdbuf();
		source = std::move(code.str());

		return true;
	}

	bool Shader::compile() {
#if defined(_DEBUG) && defined(DEBUG_SHADER)
		const char* shaderType;
		if (type == GL_VERTEX_SHADER) shaderType = "GL_VERTEX_SHADER";
		else if (type == GL_FRAGMENT_SHADER) shaderType = "GL_FRAGMENT_SHADER";
		else if (type == GL_GEOMETRY_SHADER) shaderType = "GL_GEOMETRY_SHADER";
		else if (type == GL_TESS_CONTROL_SHADER) shaderType = "GL_TESS_CONTROL_SHADER";
		else if (type == GL_TESS_EVALUATION_SHADER) shaderType = "GL_TESS_EVALUATION_SHADER";
		else if (type == GL_COMPUTE_SHADER) shaderType = "GL_COMPUTE_SHADER";
		else shaderType = "<unknown shader type>";
		std::cout << "> trying to compile a " << shaderType << " shader" << std::endl;
#endif
		char* const src = const_cast<char*>(source.c_str());
		glShaderSource(name, 1, &src, NULL);
		glCompileShader(name);

		GLint success;
		glGetShaderiv(name, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE) {
			GLint logSize;
			glGetShaderiv(name, GL_INFO_LOG_LENGTH, &logSize);

			errorLog.resize(logSize);
			glGetShaderInfoLog(name, logSize, NULL, const_cast<char*>(errorLog.c_str()));
#ifdef _DEBUG
			std::cerr << "> shader compile error: " << errorLog;
			std::cerr << source << endl;
#endif
			return false;
		}
		return true;
	}

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
	
	void VertexShaderSource::outVar(const string& type, const string& name) {
		ShaderVariable var(type, name, 0);
		COLLISION coll = collide(outVars, var);
		if(coll == COLLISION::COLLIDE) throw ("VertexShaderSource::outVar() - variable already defined with different type or length");
		if(coll == COLLISION::NO_COLLISION) outVars.push_back(var);
	}
	void VertexShaderSource::outVar(const string& type, const string& name, unsigned int length) {
		if(length == 0) throw ("VertexShaderSource::outVar() - length cannot be 0");
		ShaderVariable var(type, name, length);
		COLLISION coll = collide(outVars, var);
		if (coll == COLLISION::COLLIDE) throw ("VertexShaderSource::outVar() - variable already defined with different type or length");
		if (coll == COLLISION::NO_COLLISION) outVars.push_back(var);
	}
	void VertexShaderSource::uniformMat4(const string& name) { outVar("mat4", name); }
	void VertexShaderSource::uniform(const string& type, const string& name) {
		ShaderVariable var(type, name, 0);
		COLLISION coll = collide(uniforms, var);
		if (coll == COLLISION::COLLIDE) throw ("VertexShaderSource::uniform() - variable already defined with different type");
		if(coll == COLLISION::NO_COLLISION) uniforms.push_back(var);
	}
	void VertexShaderSource::uniform(const string& type, const string& name, unsigned int length) {
		if (length == 0) throw ("VertexShaderSource::uniform() - length cannot be 0");
		ShaderVariable var(type, name, length);
		COLLISION coll = collide(uniforms, var);
		if (coll == COLLISION::COLLIDE) throw ("VertexShaderSource::uniform() - variable already defined with different type");
		if (coll == COLLISION::NO_COLLISION) uniforms.push_back(var);
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
		std::sort(outVars.begin(), outVars.end(), svarComp);

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
				src << "  " << it->getDecl() << ";\n";
			}
			src << "} vs_out;" << std::endl;
		}
		src << "uniform mat4 modelTransform;" << std::endl;
		src << "uniform mat4 mvpTransform;" << std::endl;
		for (auto it = uniforms.begin(); it != uniforms.end(); it++) {
			src << "uniform " << it->getDecl() << ";\n";
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
		std::sort(inVars.begin(), inVars.end(), svarComp);

		for (auto it = uniforms.begin(); it != uniforms.end(); ++it) {
			src << "uniform " << it->getDecl() << ";\n";
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
				src << "  " << it->getDecl() << ";\n";
			}
			src << "} fs_in;\n";
		}
		for (auto it = outVars.begin(); it != outVars.end(); it++) {
			src << "out " << it->getDecl() << ";\n";
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
	void FragmentShaderSource::textureSampler(const string& samplerName) { uniform("sampler2D", samplerName); }
	void FragmentShaderSource::textureSamplerCube(const string& samplerName) { uniform("samplerCube", samplerName); }
	void FragmentShaderSource::textureSamplerShadow(const string& samplerName) { uniform("sampler2DShadow", samplerName); }
	void FragmentShaderSource::textureSamplerCubeShadow(const string& samplerName) { uniform("samplerCubeShadow", samplerName); }
	void FragmentShaderSource::uniform(const string& type, const string& name) {
		ShaderVariable var(type, name, 0);
		COLLISION coll = collide(uniforms, var);
		if (coll == COLLISION::COLLIDE) throw ("FragmentShaderSource::uniform() - variable already defined with different type or length");
		if (coll == COLLISION::NO_COLLISION) uniforms.push_back(var);
	}
	void FragmentShaderSource::uniform(const string& type, const string& name, unsigned int length) {
		if (length == 0) throw("FragmentShaderSource::uniform() - length cannot be 0");
		ShaderVariable var(type, name, length);
		COLLISION coll = collide(uniforms, var);
		if (coll == COLLISION::COLLIDE) throw ("FragmentShaderSource::uniform() - variable already defined with different type or length");
		if (coll == COLLISION::NO_COLLISION) uniforms.push_back(var);
	}
	void FragmentShaderSource::inVar(const string& type, const string& name) {
		ShaderVariable var(type, name, 0);
		COLLISION coll = collide(inVars, var);
		if (coll == COLLISION::COLLIDE) throw ("FragmentShaderSource::inVar() - variable already defined with different type or length");
		if (coll == COLLISION::NO_COLLISION) inVars.push_back(var);
	}
	void FragmentShaderSource::inVar(const string& type, const string& name, unsigned int length) {
		if (length == 0) throw("FragmentShaderSource::inVar() - length cannot be 0");
		ShaderVariable var(type, name, length);
		COLLISION coll = collide(inVars, var);
		if (coll == COLLISION::COLLIDE) throw ("FragmentShaderSource::inVar() - variable already defined with different type or length");
		if (coll == COLLISION::NO_COLLISION) inVars.push_back(var);
	}
	void FragmentShaderSource::outVar(const string& type, const string& name) {
		ShaderVariable var(type, name, 0);
		COLLISION coll = collide(inVars, var);
		if (coll == COLLISION::COLLIDE) throw ("FragmentShaderSource::outVar() - variable already defined with different type");
		if (coll == COLLISION::NO_COLLISION) outVars.push_back(var);
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