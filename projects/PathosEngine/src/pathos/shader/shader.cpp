// Pathos
#include "shader.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

// STL & CRT
#include <tuple>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h>

// on/off console output
#define DEBUG_SHADER			0
#define DEBUG_SHADER_SOURCE		0

#define DUMP_SHADER_SOURCE      0

// Inefficient in current design so enabling this is not recommended.
#define CONDITIONAL_COMPILE		0
#define PARSE_INCLUDES_IN_GLSL	1

using namespace std;

namespace pathos {

	GLuint createProgram(const std::string& vsCode, const std::string& fsCode, const char* debugName) {
		Shader* vshader = new Shader(GL_VERTEX_SHADER);
		Shader* fshader = new Shader(GL_FRAGMENT_SHADER);
		vshader->setSource(vsCode);
		fshader->setSource(fsCode);
		std::vector<Shader*> shaders = { vshader, fshader };

		GLuint program = createProgram(shaders, debugName);
		delete vshader;
		delete fshader;

		return program;
	}

	GLuint createProgram(Shader& vs, Shader& fs, const char* debugName) {
		std::vector<Shader*> shaders = { &vs, &fs };
		GLuint program = createProgram(shaders, debugName);
		return program;
	}

	GLuint createProgram(std::vector<ShaderSource*>& sources, const char* debugName) {
		vector<Shader*> shaders(sources.size(), nullptr);
		for (size_t i = 0; i < sources.size(); ++i) {
			ShaderSource* it = sources[i];
			Shader* shader = shaders[i] = new Shader(it->getShaderType());
			shader->setSource(it->getCode());
		}

		GLuint program = createProgram(shaders, debugName);
		for (Shader* shader : shaders) delete shader;

		return program;
	}

	GLuint createProgram(Shader& shader, const char* debugName) {
		std::vector<Shader*> shaders{ &shader };
		return createProgram(shaders, debugName);
	}

	// CAUTION: This function does not deallocate Shader objects. Delete them yourself!
	GLuint createProgram(std::vector<Shader*>& shaders, const char* debugName) {
#if DEBUG_SHADER
		LOG(LogDebug, "Create a shader program (debugName=%s)", debugName);
#endif
		for (Shader* shader : shaders) {
			bool compiled = shader->compile();
			if (!compiled) {
				if (debugName) {
					LOG(LogError, "[%s] Shader compilation error: %s", debugName, shader->getErrorLog());
				} else {
					LOG(LogError, "Shader compilation error: %s", shader->getErrorLog());
				}
				break;
			}
		}
#if DEBUG_SHADER
		LOG(LogDebug, "Linking the program...");
#endif
		GLuint program = glCreateProgram();
		if (debugName) {
			glObjectLabel(GL_PROGRAM, program, -1, debugName);
		}
		for (Shader* shader : shaders) {
			glAttachShader(program, shader->getName());
		}
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			if (maxLength == 0) maxLength = 1024;
			vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

#if _DEBUG
			std::string infoLogStr(infoLog.begin(), infoLog.end());
			LOG(LogError, "program link error: %s", infoLogStr.c_str());
			//for (ShaderSource* it : shaders) cout << it->getCode() << endl;
			LOG(LogError, "link error code: %d", glGetError());
			LOG(LogError, "program was not create, return NULL");
#endif

			glDeleteProgram(program);
			return 0;
		}
#if DEBUG_SHADER
		LOG(LogDebug, "> Finish shader program=%d debugName=%s", program, debugName);
#endif
		return program;
	}

	GLuint createComputeProgram(const std::string& shader_source, const char* debugName) {
		Shader* cs = new Shader(GL_COMPUTE_SHADER);
		cs->setSource(shader_source);
		std::vector<Shader*> shaders = { cs };

		GLuint program = createProgram(shaders, debugName);
		delete cs;

		return program;
	}

	void dumpShaderSource(ShaderSource& shader, const char* filename)
	{
#if DUMP_SHADER_SOURCE
		std::fstream file;
		file.open(filename, std::ios::out | std::ios::trunc);
		if (file.is_open() == false) {
			__debugbreak();
		}
		file << shader.getCode().data();
		file.close();
#endif
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	static bool svarComp(const ShaderVariable& a, const ShaderVariable& b) { return a.name < b.name; }
	static bool varComp(const pair<string, string>& a, const pair<string, string>& b) { return a.second < b.second; }

	enum class COLLISION { NO_COLLISION, ALREADY_EXIST, COLLIDE };
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

	Shader::Shader(GLenum inType, const char* debugName) {
		glName = glCreateShader(inType);
		type = inType;
		if (debugName) {
			glObjectLabel(GL_SHADER, glName, -1, debugName);
		}
	}

	Shader::~Shader() {
		glDeleteShader(glName);
	}

	void Shader::setSource(const std::string& newSource) { source = { newSource }; }
	void Shader::setSource(const char* newSource) { source = { newSource }; }

	bool Shader::loadSource(const std::string& filepath) { return loadSource(filepath.c_str()); }
	bool Shader::loadSource(const char* filepath_) {
		std::string filepath = ResourceFinder::get().find(filepath_);
		assert(filepath.size() > 0);

		std::ifstream file(filepath);
		if (!file.is_open()) {
			LOG(LogError, "Couldn't open a shader file: %s", filepath.c_str());
			return false;
		}

		std::ostringstream codestream;
		codestream << file.rdbuf();
		std::string fullcode = std::move(codestream.str());

		source.clear();

#if PARSE_INCLUDES_IN_GLSL
		size_t find_offset = 0u;
		while (true) {
			size_t include_start = fullcode.find("#include", find_offset);
			if (include_start == string::npos) {
				break;
			}

			size_t include_end = fullcode.find_first_of('\n', include_start);
			source.emplace_back(fullcode.substr(0, include_start));
			std::string include_line = fullcode.substr(include_start, include_end - include_start);

			size_t quote_start = include_line.find('"');
			size_t quote_end = include_line.find('"', quote_start + 1);
			assert(quote_start != string::npos && quote_end != string::npos);

			// TODO: Support recursive #include? necessary?
			std::string include_file = include_line.substr(quote_start + 1, quote_end - quote_start - 1);
			std::string include_filepath = ResourceFinder::get().find(include_file);
			std::ifstream subfile(include_filepath);
			if (!subfile.is_open()) {
				LOG(LogError, "Couldn't open a #include file: %s", include_filepath.c_str());
				return false;
			}
			std::ostringstream substream;
			substream << subfile.rdbuf();
			source.emplace_back(substream.str());

			fullcode = fullcode.substr(include_end + 1);
		}
		source.emplace_back(fullcode);
#endif

		return true;
	}

	bool Shader::compile() {
#if DEBUG_SHADER
		const char* shaderType;
		if (type == GL_VERTEX_SHADER) shaderType = "GL_VERTEX_SHADER";
		else if (type == GL_FRAGMENT_SHADER) shaderType = "GL_FRAGMENT_SHADER";
		else if (type == GL_GEOMETRY_SHADER) shaderType = "GL_GEOMETRY_SHADER";
		else if (type == GL_TESS_CONTROL_SHADER) shaderType = "GL_TESS_CONTROL_SHADER";
		else if (type == GL_TESS_EVALUATION_SHADER) shaderType = "GL_TESS_EVALUATION_SHADER";
		else if (type == GL_COMPUTE_SHADER) shaderType = "GL_COMPUTE_SHADER";
		else shaderType = "<unknown shader type>";
		LOG(LogDebug, "Trying to compille a %s", shaderType);

	#if DEBUG_SHADER_SOURCE
		std::string source_dump;
		for (const std::string& item : source) source_dump += item;

		ShaderLogFile.write(shaderType);
		ShaderLogFile.write(source_dump.c_str());
	#endif

#endif

#if CONDITIONAL_COMPILE
		char* shaderDefine = "";
		if (type == GL_VERTEX_SHADER) shaderDefine = "#define GL_VERTEX_SHADER 1\n";
		else if (type == GL_FRAGMENT_SHADER) shaderDefine = "#define GL_FRAGMENT_SHADER 1\n";
		else if (type == GL_GEOMETRY_SHADER) shaderDefine = "#define GL_GEOMETRY_SHADER 1\n";
		else if (type == GL_TESS_CONTROL_SHADER) shaderDefine = "#define GL_TESS_CONTROL_SHADER 1\n";
		else if (type == GL_TESS_EVALUATION_SHADER) shaderDefine = "#define GL_TESS_EVALUATION_SHADER 1\n";
		else if (type == GL_COMPUTE_SHADER) shaderDefine = "#define GL_COMPUTE_SHADER 1\n";

		size_t linebreak = source.find_first_of('\n', source.find("#version"));
		std::string versionDefine = source.substr(0, linebreak + 1);
		std::string tail = source.substr(linebreak + 1);
		std::string finalSource = versionDefine + shaderDefine + tail;
		char* src = const_cast<char*>(finalSource.c_str());
#else
		std::vector<const char*> src;
		for (const auto& p : source) {
			src.push_back(p.c_str());
		}
#endif

		glShaderSource(glName, (GLsizei)src.size(), src.data(), NULL); // NULL means null-terminated.
		glCompileShader(glName);

		GLint success;
		glGetShaderiv(glName, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE) {
			GLint logSize;
			glGetShaderiv(glName, GL_INFO_LOG_LENGTH, &logSize);

			errorLog.resize(logSize);
			glGetShaderInfoLog(glName, logSize, NULL, const_cast<char*>(errorLog.c_str()));
#ifdef _DEBUG
			LOG(LogError, "shader compiler error: %s", errorLog);
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
		return gcode;
	}

}
