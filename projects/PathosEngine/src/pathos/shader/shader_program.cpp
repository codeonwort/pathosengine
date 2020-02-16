#include "shader_program.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"

namespace pathos {

	ShaderProgram::ShaderProgram(const char* inDebugName)
		: debugName(inDebugName)
		, programName(0xffffffff)
	{
	}

	ShaderProgram::~ShaderProgram()
	{
		CHECK(isValid());
	}

	void ShaderProgram::reload()
	{
		GLuint oldProgramName = programName;
		bool oldValid = isValid();

		// Compile shader stages
		bool allCompiled = true;
		for (ShaderStage* shaderStage : shaderStages) {
			allCompiled = allCompiled && shaderStage->compile();
		}
		if (!allCompiled) {
			return;
		}

		// Attach and link shader stages
		programName = glCreateProgram();
		glObjectLabel(GL_PROGRAM, programName, -1, debugName);
		for (ShaderStage* shaderStage : shaderStages) {
			glAttachShader(programName, shaderStage->getGLName());
		}
		glLinkProgram(programName);

		GLint isLinked = 0;
		glGetProgramiv(programName, GL_LINK_STATUS, &isLinked);

		if (isLinked == GL_FALSE) {
			// If linking failed, leave the old program as is.
			GLint maxLength = 0;
			glGetProgramiv(programName, GL_INFO_LOG_LENGTH, &maxLength);
			if (maxLength == 0) maxLength = 1024;
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(programName, maxLength, &maxLength, infoLog.data());

			LOG(LogError, "program link error: %s", infoLog.data());
			LOG(LogError, "link error code: %d", glGetError());
			LOG(LogError, "program was not create, return NULL");

			glDeleteProgram(programName);
			programName = oldProgramName;
		} else {
			// If linked, replace old program with new one.
			if (oldValid) {
				glDeleteProgram(oldProgramName);
			}
		}
	}

	////////////////////////////////////////////////////////////

	ShaderStage::ShaderStage(GLuint inStageName, const char* inDebugName)
		: stageName(inStageName)
		, debugName(inDebugName)
	{
	}

	ShaderStage::~ShaderStage()
	{
	}

}
