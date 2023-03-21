#pragma once

#include "pathos/rhi/gl_handles.h"
#include "badger/types/string_hash.h"

#include <string>
#include <vector>
#include <map>

namespace pathos {

	// Forward decl.
	class ShaderProgram;
	class ShaderStage;

	// Needed to traverse shader programs for runtime reload.
	class ShaderDB final {
		friend class OpenGLDevice;
	
	public:
		static ShaderDB& get() {
			static ShaderDB instance;
			return instance;
		}
		
		void registerProgram(uint32 programHash, ShaderProgram* program) {
			CHECK(programMap.find(programHash) == programMap.end());
			programMap.insert(std::make_pair(programHash, program));
		}
		
		void unregisterProgram(uint32 programHash) {
			programMap.erase(programHash);
		}

		ShaderProgram* findProgram(uint32 programHash);

		void forEach(std::function<void(ShaderProgram*)> handler) {
			for(const auto& item : programMap) {
				handler(item.second);
			}
		}
		
	private:
		std::map<uint32, ShaderProgram*> programMap;
	};

	// Shader program compiled from shader stage sources.
	class ShaderProgram {

	public:
		ShaderProgram(const char* inDebugName, uint32 inProgramHash, bool inIsMaterialProgram = false);
		virtual ~ShaderProgram();

		void addShaderStage(ShaderStage* shaderStage);

		void reload();

		inline bool isValid() const { return glName != 0 && glName != 0xffffffff; }
		inline GLuint getGLName() const { return glName; }

	// messy
	public:
		void checkFirstLoad();
		bool internal_justInstantiated;

	private:
		const char* debugName;
		uint32 programHash;

		GLuint glName;

		std::vector<ShaderStage*> shaderStages;

		bool firstLoad;
		bool isMaterialProgram;
	};

	// Represents one of VS, GS, TES, TCS, or FS.
	class ShaderStage {
		friend class ShaderProgram;

	public:
		enum class CompileResponse : uint8 {
			Failed,
			NotChanged,
			Compiled
		};

	public:
		ShaderStage(GLenum inShaderType, const char* inDebugName);
		virtual ~ShaderStage();

		// Child classes override this method to call addDefine() and setFilepath()
		virtual void construct() {}

		// For material shaders
		void setSourceCode(const std::string& inFilepath, std::vector<std::string>&& inSourceCode);

	protected:
		// For global shaders
		inline void addDefine(const char* define) { defines.push_back(define); }
		inline void addDefine(const std::string& define) { defines.push_back(define); }
		void addDefine(const char* define, int32 value);
		inline void setFilepath(const char* inFilepath) { filepath = inFilepath; }

	public:
		// #todo-material-assembler: This util should not belong here.
		static bool loadSourceInternal(
			const std::string& filepath,
			const std::vector<std::string>& defines,
			int32 recursionDepth,
			std::vector<std::string>& includeHistory,
			std::vector<std::string>& outSourceCode);
	private:
		bool loadSource();
		ShaderStage::CompileResponse tryCompile(const char* programName, bool checkSourceChanges);
		bool finishCompile();

		inline GLuint getGLName() const { return glName; }

	private:
		GLenum shaderType;
		const char* debugName;

		GLuint glName;
		GLuint pendingGLName;
		
		std::string filepath;
		std::vector<std::string> defines;
		std::vector<std::string> sourceCode;

	};

	// #todo-shader: Serious problem - now I can't destroy shader programs until application termination.
#define DEFINE_COMPUTE_PROGRAM(ShaderProgramClass, ComputeStageClass)                                \
	static constexpr uint32 ShaderProgramClass##_Hash = COMPILE_TIME_CRC32_STR(#ShaderProgramClass); \
	class ShaderProgramClass : public ShaderProgram {                                                \
	public:                                                                                          \
		static ShaderProgramClass& get() {                                                           \
			static ShaderProgramClass instance(#ShaderProgramClass, ShaderProgramClass##_Hash);      \
			if (instance.internal_justInstantiated) {                                                \
				instance.internal_justInstantiated = false;                                          \
				instance.addShaderStage(new ComputeStageClass);                                      \
			}                                                                                        \
			instance.checkFirstLoad();                                                               \
			return instance;                                                                         \
		}                                                                                            \
		ShaderProgramClass(const char* inDebugName, uint32 inProgramHash)                            \
			: ShaderProgram(inDebugName, inProgramHash) {}                                           \
	};                                                                                               \

#define DEFINE_SHADER_PROGRAM2(ShaderProgramClass, Stage1, Stage2)                                   \
	static constexpr uint32 ShaderProgramClass##_Hash = COMPILE_TIME_CRC32_STR(#ShaderProgramClass); \
	class ShaderProgramClass : public ShaderProgram {                                                \
	public:                                                                                          \
		static ShaderProgramClass& get() {                                                           \
			static ShaderProgramClass instance(#ShaderProgramClass, ShaderProgramClass##_Hash);      \
			if (instance.internal_justInstantiated) {                                                \
				instance.internal_justInstantiated = false;                                          \
				instance.addShaderStage(new Stage1);                                                 \
				instance.addShaderStage(new Stage2);                                                 \
			}                                                                                        \
			instance.checkFirstLoad();                                                               \
			return instance;                                                                         \
		}                                                                                            \
		ShaderProgramClass(const char* inDebugName, uint32 inProgramHash)                            \
			: ShaderProgram(inDebugName, inProgramHash) {}                                           \
	};                                                                                               \

#define DEFINE_SHADER_PROGRAM3(ShaderProgramClass, Stage1, Stage2, Stage3)                           \
	static constexpr uint32 ShaderProgramClass##_Hash = COMPILE_TIME_CRC32_STR(#ShaderProgramClass); \
	class ShaderProgramClass : public ShaderProgram {                                                \
	public:                                                                                          \
		static ShaderProgramClass& get() {                                                           \
			static ShaderProgramClass instance(#ShaderProgramClass, ShaderProgramClass##_Hash);      \
			if (instance.internal_justInstantiated) {                                                \
				instance.internal_justInstantiated = false;                                          \
				instance.addShaderStage(new Stage1);                                                 \
				instance.addShaderStage(new Stage2);                                                 \
				instance.addShaderStage(new Stage3);                                                 \
			}                                                                                        \
			instance.checkFirstLoad();                                                               \
			return instance;                                                                         \
		}                                                                                            \
		ShaderProgramClass(const char* inDebugName, uint32 inProgramHash)                            \
			: ShaderProgram(inDebugName, inProgramHash) {}                                           \
	};                                                                                               \

#define DEFINE_SHADER_PROGRAM4(ShaderProgramClass, Stage1, Stage2, Stage3, Stage4)                   \
	static constexpr uint32 ShaderProgramClass##_Hash = COMPILE_TIME_CRC32_STR(#ShaderProgramClass); \
	class ShaderProgramClass : public ShaderProgram {                                                \
	public:                                                                                          \
		static ShaderProgramClass& get() {                                                           \
			static ShaderProgramClass instance(#ShaderProgramClass, ShaderProgramClass##_Hash);      \
			if (instance.internal_justInstantiated) {                                                \
				instance.internal_justInstantiated = false;                                          \
				instance.addShaderStage(new Stage1);                                                 \
				instance.addShaderStage(new Stage2);                                                 \
				instance.addShaderStage(new Stage3);                                                 \
				instance.addShaderStage(new Stage4);                                                 \
			}                                                                                        \
			instance.checkFirstLoad();                                                               \
			return instance;                                                                         \
		}                                                                                            \
		ShaderProgramClass(const char* inDebugName, uint32 inProgramHash)                            \
			: ShaderProgram(inDebugName, inProgramHash) {}                                           \
	};                                                                                               \

#define DEFINE_SHADER_PROGRAM5(ShaderProgramClass, Stage1, Stage2, Stage3, Stage4, Stage5)           \
	static constexpr uint32 ShaderProgramClass##_Hash = COMPILE_TIME_CRC32_STR(#ShaderProgramClass); \
	class ShaderProgramClass : public ShaderProgram {                                                \
	public:                                                                                          \
		static ShaderProgramClass& get() {                                                           \
			static ShaderProgramClass instance(#ShaderProgramClass, ShaderProgramClass##_Hash);      \
			if (instance.internal_justInstantiated) {                                                \
				instance.internal_justInstantiated = false;                                          \
				instance.addShaderStage(new Stage1);                                                 \
				instance.addShaderStage(new Stage2);                                                 \
				instance.addShaderStage(new Stage3);                                                 \
				instance.addShaderStage(new Stage4);                                                 \
				instance.addShaderStage(new Stage5);                                                 \
			}                                                                                        \
			instance.checkFirstLoad();                                                               \
			return instance;                                                                         \
		}                                                                                            \
		ShaderProgramClass(const char* inDebugName, uint32 inProgramHash)                            \
			: ShaderProgram(inDebugName, inProgramHash) {}                                           \
	};                                                                                               \

#define FIND_SHADER_PROGRAM(ShaderProgramClass) ShaderProgramClass::get()

}
