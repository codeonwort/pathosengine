#include "shader_program.h"
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
		if (isValid()) {
			glDeleteProgram(programName);
			programName = 0xffffffff;
		}

		// #todo-shader-rework: Reload stages and recreate GL program.
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
