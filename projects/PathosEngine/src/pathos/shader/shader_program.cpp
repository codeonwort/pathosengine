#include "shader_program.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

#define DUMP_SHADER_SOURCE 0

namespace pathos {

	ShaderProgram::ShaderProgram(const char* inDebugName)
		: debugName(inDebugName)
		, glName(0xffffffff)
	{
	}

	ShaderProgram::~ShaderProgram()
	{
		CHECK(isValid());
	}

	void ShaderProgram::reload()
	{
		GLuint oldGLName = glName;
		bool oldValid = isValid();

		// Try to compile shader stages
		bool allCompiled = true;
		for (ShaderStage* shaderStage : shaderStages) {
			allCompiled = allCompiled && shaderStage->tryCompile();
		}
		if (!allCompiled) {
			return;
		}

		// Finalize shader compilation
		for (ShaderStage* shaderStage : shaderStages) {
			shaderStage->finishCompile();
		}

		// Attach and link shader stages
		glName = glCreateProgram();
		glObjectLabel(GL_PROGRAM, glName, -1, debugName);
		for (ShaderStage* shaderStage : shaderStages) {
			glAttachShader(glName, shaderStage->getGLName());
		}
		glLinkProgram(glName);

		GLint isLinked = 0;
		glGetProgramiv(glName, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_FALSE) {
			// If linking failed, leave the old program as is.
			GLint maxLength = 0;
			glGetProgramiv(glName, GL_INFO_LOG_LENGTH, &maxLength);
			if (maxLength == 0) maxLength = 1024;
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(glName, maxLength, &maxLength, infoLog.data());

			LOG(LogError, "program link error: %s", infoLog.data());
			LOG(LogError, "link error code: %d", glGetError());
			LOG(LogError, "program was not created, return NULL");

			glDeleteProgram(glName);
			glName = oldGLName;
		} else {
			// If linked, replace old program with new one.
			if (oldValid) {
				glDeleteProgram(oldGLName);
			}
		}
	}

	////////////////////////////////////////////////////////////

	ShaderStage::ShaderStage(GLenum inShaderType, const char* inDebugName)
		: shaderType(inShaderType)
		, debugName(inDebugName)
	{
		CHECK( inShaderType == GL_VERTEX_SHADER
			|| inShaderType == GL_GEOMETRY_SHADER
			|| inShaderType == GL_TESS_CONTROL_SHADER
			|| inShaderType == GL_TESS_EVALUATION_SHADER
			|| inShaderType == GL_FRAGMENT_SHADER
			|| inShaderType == GL_COMPUTE_SHADER);
		CHECK(inDebugName != nullptr);

		glName = glCreateShader(shaderType);
		glObjectLabel(GL_SHADER, glName, -1, inDebugName);
		pendingGLName = 0;

		filepath = nullptr;
	}

	ShaderStage::~ShaderStage()
	{
		glDeleteShader(glName);
	}

	bool ShaderStage::loadSource()
	{
		// todo
	}

	bool ShaderStage::tryCompile()
	{
		if (pendingGLName != 0) {
			glDeleteShader(pendingGLName);
		}

		// #todo-shader-rework: Output shader code to intermediate/shader_dump
#if DUMP_SHADER_SOURCE
		//
#endif
		
		std::vector<const char*> sourceList;
		for (const auto& s : sourceCode) {
			sourceList.push_back(s.c_str());
		}

		pendingGLName = glCreateShader(shaderType);
		glShaderSource(pendingGLName, (GLsizei)sourceList.size(), sourceList.data(), NULL);
		glCompileShader(pendingGLName);

		GLint success;
		glGetShaderiv(glName, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE) {
			GLint logSize;
			glGetShaderiv(pendingGLName, GL_INFO_LOG_LENGTH, &logSize);

			std::string errorLog;
			errorLog.resize(logSize);
			glGetShaderInfoLog(pendingGLName, logSize, NULL, &errorLog[0]);
			LOG(LogError, "Failed to compile shader (%s) : %s", debugName, errorLog.c_str());

			glDeleteShader(pendingGLName);
			pendingGLName = 0;
		}

		return success;
	}

	bool ShaderStage::finishCompile()
	{
		if (pendingGLName == 0) {
			return false;
		}

		if (glName != 0) {
			glDeleteShader(glName);
		}

		glName = pendingGLName;
		pendingGLName = 0;

		return true;
	}

}
