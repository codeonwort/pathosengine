#pragma once

#include "gl_core.h"

#include <string>
#include <vector>
#include <map>

namespace pathos {

	// Forward decl.
	class ShaderProgram;
	class ShaderStage;

	// Needed to traverse shader programs for runtime reload.
	class ShaderDB final {
	
	public:
		static ShaderDB& get() {
			static ShaderDB instance;
			return instance;
		}
		
		void registerProgram(uint32 programHash, ShaderProgram* program) {
			CHECK(mapping.find(programHash) == mapping.end());
			mapping.insert(std::make_pair(programHash, program));
		}
		
		void unregisterProgram(uint32 programHash) {
			mapping.erase(programHash);
		}

		void forEach(std::function<void(ShaderProgram*)> handler) {
			for(const auto& item : mapping) {
				handler(item.second);
			}
		}
		
	private:
		std::map<uint32, ShaderProgram*> mapping;
		
	};

	// Shader program compiled from shader stage sources.
	class ShaderProgram {

	public:
		ShaderProgram(const char* inDebugName);
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
		GLuint glName;

		std::vector<ShaderStage*> shaderStages;

		bool firstLoad;

	};

	// Represents one of VS, GS, TES, TCS, or FS.
	// #todo-shader-rework: Replace pathos::Shader with this.
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

	protected:
		inline void addDefine(const char* define) { defines.push_back(define); }
		inline void addDefine(const std::string& define) { defines.push_back(define); }
		inline void setFilepath(const char* inFilepath) { filepath = inFilepath; }

	private:
		bool loadSource();
		ShaderStage::CompileResponse tryCompile();
		bool finishCompile();

		inline GLuint getGLName() const { return glName; }

	private:
		GLenum shaderType;
		const char* debugName;

		GLuint glName;
		GLuint pendingGLName;
		
		const char* filepath;
		std::vector<std::string> defines;
		std::vector<std::string> sourceCode;

	};

	// #todo-shader-rework: Code-level shader source assembler. pathos::ShaderSource was my very first attempt for this,
	// but the implementation was really cumbersome, not knowing what am I doing. After working with UE4's vertex factory
	// and material system, I think I can do better now.
	//
	// class ShaderAssembler { ... };
	//

	// #todo-shader-rework: Serious problem - now I can't destroy shader programs until application termination.
#define DEFINE_COMPUTE_PROGRAM(ShaderProgramClass, ComputeStageClass)   \
	class ShaderProgramClass : public ShaderProgram {                   \
	public:                                                             \
		static ShaderProgramClass& get() {                              \
			static ShaderProgramClass instance(#ShaderProgramClass);    \
			if (instance.internal_justInstantiated) {                   \
				instance.internal_justInstantiated = false;             \
				instance.addShaderStage(new ComputeStageClass);         \
			}                                                           \
			instance.checkFirstLoad();                                  \
			return instance;                                            \
		}                                                               \
		ShaderProgramClass(const char* inDebugName)                     \
			: ShaderProgram(inDebugName) {}                             \
	};                                                                  \

#define DEFINE_SHADER_PROGRAM2(ShaderProgramClass, Stage1, Stage2)      \
	class ShaderProgramClass : public ShaderProgram {                   \
	public:                                                             \
		static ShaderProgramClass& get() {                              \
			static ShaderProgramClass instance(#ShaderProgramClass);    \
			if (instance.internal_justInstantiated) {                   \
				instance.internal_justInstantiated = false;             \
				instance.addShaderStage(new Stage1);                    \
				instance.addShaderStage(new Stage2);                    \
			}                                                           \
			instance.checkFirstLoad();                                  \
			return instance;                                            \
		}                                                               \
		ShaderProgramClass(const char* inDebugName)                     \
			: ShaderProgram(inDebugName) {}                             \
	};                                                                  \

#define DEFINE_SHADER_PROGRAM3(ShaderProgramClass, Stage1, Stage2, Stage3) \
	class ShaderProgramClass : public ShaderProgram {                   \
	public:                                                             \
		static ShaderProgramClass& get() {                              \
			static ShaderProgramClass instance(#ShaderProgramClass);    \
			if (instance.internal_justInstantiated) {                   \
				instance.internal_justInstantiated = false;             \
				instance.addShaderStage(new Stage1);                    \
				instance.addShaderStage(new Stage2);                    \
				instance.addShaderStage(new Stage3);                    \
			}                                                           \
			instance.checkFirstLoad();                                  \
			return instance;                                            \
		}                                                               \
		ShaderProgramClass(const char* inDebugName)                     \
			: ShaderProgram(inDebugName) {}                             \
	};                                                                  \

#define DEFINE_SHADER_PROGRAM4(ShaderProgramClass, Stage1, Stage2, Stage3, Stage4) \
	class ShaderProgramClass : public ShaderProgram {                   \
	public:                                                             \
		static ShaderProgramClass& get() {                              \
			static ShaderProgramClass instance(#ShaderProgramClass);    \
			if (instance.internal_justInstantiated) {                   \
				instance.internal_justInstantiated = false;             \
				instance.addShaderStage(new Stage1);                    \
				instance.addShaderStage(new Stage2);                    \
				instance.addShaderStage(new Stage3);                    \
				instance.addShaderStage(new Stage4);                    \
			}                                                           \
			instance.checkFirstLoad();                                  \
			return instance;                                            \
		}                                                               \
		ShaderProgramClass(const char* inDebugName)                     \
			: ShaderProgram(inDebugName) {}                             \
	};                                                                  \

#define DEFINE_SHADER_PROGRAM5(ShaderProgramClass, Stage1, Stage2, Stage3, Stage4, Stage5) \
	class ShaderProgramClass : public ShaderProgram {                   \
	public:                                                             \
		static ShaderProgramClass& get() {                              \
			static ShaderProgramClass instance(#ShaderProgramClass);    \
			if (instance.internal_justInstantiated) {                   \
				instance.internal_justInstantiated = false;             \
				instance.addShaderStage(new Stage1);                    \
				instance.addShaderStage(new Stage2);                    \
				instance.addShaderStage(new Stage3);                    \
				instance.addShaderStage(new Stage4);                    \
				instance.addShaderStage(new Stage5);                    \
			}                                                           \
			instance.checkFirstLoad();                                  \
			return instance;                                            \
		}                                                               \
		ShaderProgramClass(const char* inDebugName)                     \
			: ShaderProgram(inDebugName) {}                             \
	};                                                                  \

#define FIND_SHADER_PROGRAM(ShaderProgramClass) ShaderProgramClass::get();

}
