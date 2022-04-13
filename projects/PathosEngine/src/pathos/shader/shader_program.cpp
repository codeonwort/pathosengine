#include "shader_program.h"
#include "pathos/engine.h"
#include "pathos/render/render_device.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"

#include "badger/assertion/assertion.h"

#include <fstream>
#include <sstream>

#define DUMP_SHADER_SOURCE 0
#define IGNORE_SAME_SHADERS_ON_RECOMPILE 1

namespace pathos {

	static struct InitRecompileShaders {
		InitRecompileShaders() {
			Engine::internal_registerGlobalRenderRoutine(InitRecompileShaders::recompileShaders, nullptr);
		}
		// Dirty but works anyway
		static void recompileShaders(OpenGLDevice* device, RenderCommandList& cmdList) {
			gEngine->registerExec("recompile_shaders", [](const std::string& command) -> void {
				LOG(LogInfo, "Begin reloading shaders...");
				ShaderDB::get().forEach([](ShaderProgram* program) -> void {
					program->reload();
				});
				LOG(LogInfo, "End reloading shaders.");
			});
		}
	} internal_recompileShaders;

	ShaderProgram::ShaderProgram(const char* inDebugName, uint32 inProgramHash)
		: debugName(inDebugName)
		, programHash(inProgramHash)
		, glName(0xffffffff)
		, firstLoad(true)
		, internal_justInstantiated(true)
	{
		ShaderDB::get().registerProgram(inProgramHash, this);
	}

	ShaderProgram::~ShaderProgram()
	{
		CHECK(isValid());
		ShaderDB::get().unregisterProgram(programHash);
	}

	void ShaderProgram::addShaderStage(ShaderStage* shaderStage)
	{
		shaderStages.push_back(shaderStage);
	}

	void ShaderProgram::reload()
	{
		GLuint oldGLName = glName;
		bool oldValid = isValid();

		// Try to compile shader stages
		bool allCompiled = true;
		bool allNotChanged = true;
		for (ShaderStage* shaderStage : shaderStages) {
			ShaderStage::CompileResponse response = shaderStage->tryCompile();
			allCompiled = allCompiled && response != ShaderStage::CompileResponse::Failed;
			allNotChanged = allNotChanged && response == ShaderStage::CompileResponse::NotChanged;
		}
		if (allNotChanged) {
#if IGNORE_SAME_SHADERS_ON_RECOMPILE == 0
			LOG(LogDebug, "%s: All shader stages are not changed, won't recompile.", debugName);
#endif
			return;
		}
		if (!allCompiled) {
			LOG(LogDebug, "%s: Failed to compile some shader stages.", debugName);
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

			LOG(LogError, "program link error(code=%d): %s", glGetError(), infoLog.data());

			glDeleteProgram(glName);
			glName = oldGLName;
		} else {
			// If linked, replace old program with new one.
			if (oldValid) {
				glDeleteProgram(oldGLName);
			}
			if (!firstLoad) {
				LOG(LogDebug, "%s: Recompiled the shader program.", debugName);
			}
		}
	}

	void ShaderProgram::checkFirstLoad()
	{
		if (firstLoad) {
			reload();
			firstLoad = false;
		}
	}

	////////////////////////////////////////////////////////////

	ShaderStage::ShaderStage(GLenum inShaderType, const char* inDebugName)
		: shaderType(inShaderType)
		, debugName(inDebugName)
		, glName(0)
		, pendingGLName(0)
		, filepath(nullptr)
	{
		CHECK( inShaderType == GL_VERTEX_SHADER
			|| inShaderType == GL_GEOMETRY_SHADER
			|| inShaderType == GL_TESS_CONTROL_SHADER
			|| inShaderType == GL_TESS_EVALUATION_SHADER
			|| inShaderType == GL_FRAGMENT_SHADER
			|| inShaderType == GL_COMPUTE_SHADER);
		CHECK(inDebugName != nullptr);
	}

	ShaderStage::~ShaderStage()
	{
		if (glName != 0) {
			glDeleteShader(glName);
		}
	}

	bool ShaderStage::loadSource()
	{
		CHECK(filepath != nullptr);

		std::string fullFilepath = ResourceFinder::get().find(filepath);

		if (fullFilepath.size() == 0) {
			LOG(LogError, "[%s]: Couldn't find file: %s", __FUNCTION__, filepath);
			return false;
		}

		std::ifstream fileStream(fullFilepath);
		if (fileStream.is_open() == false) {
			LOG(LogError, "[%s]: Couldn't open file: %s", __FUNCTION__, filepath);
			return false;
		}

		std::ostringstream codeStream;
		codeStream << fileStream.rdbuf();
		std::string fullCode = std::move(codeStream.str());

		size_t version_start = fullCode.find("#version");
		if (version_start == std::string::npos) {
			LOG(LogError, "[%s]: GLSL source file should contain '#version' statement", __FUNCTION__);
			return false;
		}

		size_t version_end = fullCode.find_first_of('\n', version_start);

		// Add defines
		if (defines.size() > 0) {
			codeStream.clear();
			codeStream.str("");
			// Put #version back
			codeStream << fullCode.substr(version_start, version_end - version_start + 1);
			for (const std::string& def : defines) {
				codeStream << "#define " << def << '\n';
			}
			codeStream << fullCode.substr(version_end + 1);
			fullCode = std::move(codeStream.str());
		}

		sourceCode.clear();

		size_t find_offset = 0u;
		while (true) {
			// #todo-shader: Need to skip '#include' in comments
			size_t include_start = fullCode.find("#include", find_offset);
			if (include_start == std::string::npos) {
				break;
			}

			size_t include_end = fullCode.find_first_of('\n', include_start);
			sourceCode.emplace_back(fullCode.substr(0, include_start));
			std::string include_line = fullCode.substr(include_start, include_end - include_start);

			size_t quote_start = include_line.find('"');
			size_t quote_end = include_line.find('"', quote_start + 1);
			CHECK(quote_start != std::string::npos && quote_end != std::string::npos);

			// #todo-shader: Support recursive include?
			std::string include_file = include_line.substr(quote_start + 1, quote_end - quote_start - 1);
			std::string include_filepath = ResourceFinder::get().find(include_file);
			std::ifstream subfile(include_filepath);
			if (!subfile.is_open()) {
				LOG(LogError, "Couldn't open a #include file: %s", include_filepath.c_str());
				return false;
			}
			std::ostringstream substream;
			substream << subfile.rdbuf();
			sourceCode.emplace_back(substream.str());

			fullCode = fullCode.substr(include_end + 1);
		}

		sourceCode.emplace_back(fullCode);

		return true;
	}

	ShaderStage::CompileResponse ShaderStage::tryCompile()
	{
		std::vector<std::string> sourceCodeBackup = sourceCode;
		loadSource();

		bool sourceChanged = false;
		if (sourceCode.size() == sourceCodeBackup.size()) {
			for (uint32 i = 0; i < sourceCode.size(); ++i) {
				if (sourceCode[i] != sourceCodeBackup[i]) {
					sourceChanged = true;
					break;
				}
			}
		} else {
			sourceChanged = true;
		}
		if (sourceChanged == false) {
#if IGNORE_SAME_SHADERS_ON_RECOMPILE == 0
			LOG(LogDebug, "%s: Source code is same.", debugName);
#endif
			return ShaderStage::CompileResponse::NotChanged;
		}

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
		glGetShaderiv(pendingGLName, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE) {
			GLint logSize;
			glGetShaderiv(pendingGLName, GL_INFO_LOG_LENGTH, &logSize);

			std::string errorLog;
			errorLog.resize(logSize);
			glGetShaderInfoLog(pendingGLName, logSize, NULL, &errorLog[0]);
			LOG(LogError, "Failed to compile shader (%s) : %s", debugName, errorLog.c_str());

			glDeleteShader(pendingGLName);
			pendingGLName = 0;

			return ShaderStage::CompileResponse::Failed;
		}

		return ShaderStage::CompileResponse::Compiled;
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

		glObjectLabel(GL_SHADER, glName, -1, debugName);

		return true;
	}

}
