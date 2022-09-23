// Pathos
#include "gl_core.h"
#include "shader.h"
#include "shader_program.h"
#include "pathos/util/resource_finder.h"
#include "pathos/util/log.h"

// STL & CRT
#include <tuple>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

// on/off console output
#define DEBUG_SHADER			0
#define DEBUG_SHADER_SOURCE		0

#define PARSE_INCLUDES_IN_GLSL	1


namespace pathos {

	GLuint createProgram(Shader& vs, Shader& fs, const char* debugName) {
		std::vector<Shader*> shaders = { &vs, &fs };
		GLuint program = createProgram(shaders, debugName);
		return program;
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

	bool Shader::loadSource(const std::string& filepath) {
		return loadSource(filepath.c_str());
	}
	bool Shader::loadSource(const char* filepath_) {
		CHECK(filepath_ != nullptr);

		source.clear();
		return ShaderStage::loadSourceInternal(filepath_, defines, 0, source);
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

		std::vector<const char*> src;
		for (const auto& p : source) {
			src.push_back(p.c_str());
		}

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

}
