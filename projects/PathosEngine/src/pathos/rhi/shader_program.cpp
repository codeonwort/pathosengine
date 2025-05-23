#include "shader_program.h"
#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/rhi/render_device.h"
#include "pathos/material/material_shader_assembler.h"
#include "pathos/util/log.h"
#include "pathos/util/resource_finder.h"

#include "badger/assertion/assertion.h"

#include <fstream>
#include <sstream>
#include <filesystem>

#define IGNORE_SAME_SHADERS_ON_RECOMPILE 1

namespace pathos {

	// Turn this on EngineConfig.ini as shaders are compiled on startup and you don't have a chance to change this through console.
	static ConsoleVariable<int32> cvar_dumpShaders("r.dumpShaderSources", 0, "Dump shader sources to log/shader_dump");

	static struct InitRecompileShaders {
		InitRecompileShaders() {
			Engine::internal_registerGlobalRenderRoutine(InitRecompileShaders::recompileShaders, nullptr);
		}
		// Dirty but works anyway
		static void recompileShaders(OpenGLDevice* device, RenderCommandList& cmdList) {
			gEngine->registerConsoleCommand("recompile_shaders", [](const std::string& command) -> void {
				LOG(LogInfo, "Begin reloading shaders...");
				// Process material shaders.
				MaterialShaderAssembler::get().reloadMaterialShaders();
				// Process non-material shaders.
				ENQUEUE_RENDER_COMMAND([](RenderCommandList& cmdList) {
					ShaderDB::get().forEach([](ShaderProgram* program) -> void {
						if (program->isMaterialProgram() == false) {
							program->reload();
						}
					});
				});
				FLUSH_RENDER_COMMAND();
				LOG(LogInfo, "End reloading shaders.");
			});
		}
	} internal_recompileShaders;

	ShaderProgram* ShaderDB::findProgram(uint32 programHash, bool checkCompiled) const {
		if (checkCompiled) CHECK(isInRenderThread());

		auto it = programMap.find(programHash);
		if (it != programMap.end()) {
			ShaderProgram* program = it->second;
			if (checkCompiled) program->checkFirstLoad();
			return program;
		}
		return nullptr;
	}

}

namespace pathos {

	ShaderProgram::ShaderProgram(const char* inDebugName, uint32 inProgramHash, bool inIsMaterialProgram /*= false*/)
		: debugName(inDebugName)
		, programHash(inProgramHash)
		, bIsMaterialProgram(inIsMaterialProgram)
		, glName(0xffffffff)
		, bFirstLoad(true)
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
		CHECK(isInRenderThread());

		GLuint oldGLName = glName;
		bool oldValid = isValid();

		// Try to compile shader stages
		bool allCompiled = true;
		bool allNotChanged = true;

		bool checkSourceChanges = !bIsMaterialProgram;

		for (ShaderStage* shaderStage : shaderStages) {
			ShaderStage::CompileResponse response = shaderStage->tryCompile(debugName, checkSourceChanges);
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
			if (!bFirstLoad) {
				LOG(LogDebug, "%s: Recompiled the shader program.", debugName);
			}
		}
	}

	void ShaderProgram::checkFirstLoad()
	{
		if (bFirstLoad) {
			reload();
			bFirstLoad = false;
		}
	}

	////////////////////////////////////////////////////////////

	ShaderStage::ShaderStage(GLenum inShaderType, const char* inDebugName)
		: shaderType(inShaderType)
		, debugName(inDebugName)
		, glName(0)
		, pendingGLName(0)
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
		CHECK(isInRenderThread());

		if (glName != 0) {
			glDeleteShader(glName);
		}
	}

	void ShaderStage::addDefine(const char* define, int32 value) {
		CHECK(define != nullptr);
		char msg[256];
		sprintf_s(msg, "%s %d", define, value);
		addDefine(msg);
	}

	bool ShaderStage::loadSource() {
		CHECK(filepath.c_str() != nullptr);

		sourceCode.clear();
		std::vector<std::string> includeHistory;
		return ShaderStage::loadSourceInternal(filepath, defines, 0, includeHistory, sourceCode);
	}

	void ShaderStage::setSourceCode(const std::string& inFilepath, std::vector<std::string>&& inSourceCode) {
		filepath = inFilepath;
		sourceCode = inSourceCode;
	}

	// #todo-shader: This is getting too dirty
	bool ShaderStage::loadSourceInternal(
		const std::string& filepath,
		const std::vector<std::string>& defines,
		int32 recursionDepth,
		std::vector<std::string>& includeHistory,
		std::vector<std::string>& outSourceCode)
	{
		std::string fullFilepath = ResourceFinder::get().find(filepath);
		std::filesystem::path currentDir = std::filesystem::path(fullFilepath).parent_path();

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

		if (recursionDepth == 0) {
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
		}

		// Parse include statements
		size_t find_offset = 0u;
		while (true) {
			size_t include_start = fullCode.find("#include", find_offset);
			if (include_start == std::string::npos) {
				break;
			}

			bool isComment = false;
			size_t include_line_start = include_start;
			while (include_line_start > 0 && fullCode[include_line_start - 1] != '\n') {
				include_line_start -= 1;
			}
			// Same as std::isspace() https://en.cppreference.com/w/cpp/string/byte/isspace
			include_line_start = fullCode.find_first_not_of(" \t\n\r\f\v", include_line_start);
			isComment = (fullCode[include_line_start] == '/' && fullCode[include_line_start + 1] == '/');

			size_t include_end = fullCode.find_first_of('\n', include_start);

			if (isComment) {
				outSourceCode.emplace_back(fullCode.substr(0, include_end));
				fullCode = fullCode.substr(include_end + 1);
				continue;
			}
			
			outSourceCode.emplace_back(fullCode.substr(0, include_start));
			std::string include_line = fullCode.substr(include_start, include_end - include_start);

			size_t quote_start = include_line.find('"');
			size_t quote_end = include_line.find('"', quote_start + 1);
			CHECK(quote_start != std::string::npos && quote_end != std::string::npos);

			std::string includeRel = include_line.substr(quote_start + 1, quote_end - quote_start - 1);

			std::filesystem::path relPath = currentDir;
			relPath.append(includeRel);
			std::string includeFull;
			if (std::filesystem::exists(relPath)) {
				includeFull = std::filesystem::canonical(relPath).string();
			} else {
				includeFull = ResourceFinder::get().find(includeRel);
			}

			if (includeFull.size() == 0) {
				LOG(LogError, "Couldn't open %s in %s", includeRel.c_str(), fullFilepath.c_str());
			} else if (std::find(includeHistory.begin(), includeHistory.end(), includeFull) == includeHistory.end()) {
				includeHistory.push_back(includeFull);
				ShaderStage::loadSourceInternal(includeFull, defines, recursionDepth + 1, includeHistory, outSourceCode);
			}

			fullCode = fullCode.substr(include_end + 1);
		}

		outSourceCode.emplace_back(fullCode);

		return true;
	}

	ShaderStage::CompileResponse ShaderStage::tryCompile(const char* programName, bool checkSourceChanges) {
		if (checkSourceChanges) {
			std::vector<std::string> sourceCodeBackup = sourceCode;
			loadSource();

			bool sourceChanged = sourceCodeBackup != sourceCode;
			if (sourceChanged == false) {
#if IGNORE_SAME_SHADERS_ON_RECOMPILE == 0
				LOG(LogDebug, "%s: Source code is same.", debugName);
#endif
				return ShaderStage::CompileResponse::NotChanged;
			}
		}

		if (pendingGLName != 0) {
			glDeleteShader(pendingGLName);
		}

		// Dump final shader source to log/shader_dump/
		if (cvar_dumpShaders.getInt() != 0) {
			std::string basedir = pathos::getSolutionDir();
			basedir += "log/shader_dump/";
			pathos::createDirectory(basedir.c_str());

			if (programName != nullptr) {
				basedir += programName;
				basedir += "/";
				pathos::createDirectory(basedir.c_str());
			}

			std::string dumpPath = basedir + filepath;

			std::fstream fs(dumpPath, std::fstream::out);
			if (fs.is_open()) {
				for (uint32 i = 0; i < sourceCode.size(); ++i) {
					const std::string& s = sourceCode[i];
					fs << s;
					if (s.size() == 0 || s[s.size()-1] != '\n') {
						fs << std::endl;
					}
				}
				fs.close();
			}
		}
		
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
			
			if (cvar_dumpShaders.getInt() != 0 && programName != nullptr) {
				std::string basedir = pathos::getSolutionDir();
				basedir += "log/shader_dump/";
				basedir += programName;
				basedir += "/";
				pathos::createDirectory(basedir.c_str());

				std::string dumpPath = basedir + "error.txt";

				std::fstream fs(dumpPath, std::fstream::out);
				if (fs.is_open()) {
					fs << errorLog << std::endl;
					fs.close();
				}
			}

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
