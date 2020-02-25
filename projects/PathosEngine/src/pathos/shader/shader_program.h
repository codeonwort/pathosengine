#pragma once

#include "gl_core.h"

#include <string>
#include <vector>
#include <map>

namespace pathos {

	// Forward decl.
	class ShaderProgram;
	class ShaderStage;

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

		ShaderProgram* find(const char* programClassName);

	private:
		std::map<uint32, ShaderProgram*> mapping;

	};

	// Shader program compiled from shader stage sources.
	// #todo-shader-rework: When to initialize GL programs? at demand or on startup, or support both ways?
	class ShaderProgram final {

	public:
		static bool shaderDB_initialized;
		static std::map<uint32, ShaderProgram*> shaderDB;

	public:
		ShaderProgram(const char* inDebugName);
		~ShaderProgram();

		void addShaderStage(ShaderStage* shaderStage);

		void reload();

		void checkFirstLoad();

		inline bool isValid() const { return glName != 0 && glName != 0xffffffff; }
		inline GLuint getGLName() const { return glName; }

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
		ShaderStage(GLenum inShaderType, const char* inDebugName);
		virtual ~ShaderStage();

		// Child classes override this method to call addDefine() and setFilepath()
		virtual void construct() = 0;

	protected:
		inline void addDefine(const char* define) { defines.push_back(define); }
		inline void addDefine(const std::string& define) { defines.push_back(define); }
		inline void setFilepath(const char* inFilepath) { filepath = inFilepath; }

	private:
		bool loadSource();
		bool tryCompile();
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


	// #todo-shader-rework: What the hell am I doing with my life?
	template<typename... StageClasses>
	struct ShaderProgramInitializer {
		ShaderProgramInitializer(const char* programName)
			: program(programName)
		{
			addStage<StageClasses...>();
		}
	private:
		// CS
		template<typename Stage1>
		void addStage() {
			static_assert(std::is_base_of<ShaderStage, Stage1>::value, "Not derived from pathos::ShaderStage");
			program.addShaderStage(new Stage1);
		}

		// VS, PS
		template<typename Stage1, typename Stage2>
		void addStage() {
			static_assert(std::is_base_of<ShaderStage, Stage1>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage2>::value, "Not derived from pathos::ShaderStage");
			program.addShaderStage(new Stage1);
			program.addShaderStage(new Stage2);
		}

		// VS, GS, PS
		template<typename Stage1, typename Stage2, typename Stage3>
		void addStage() {
			static_assert(std::is_base_of<ShaderStage, Stage1>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage2>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage3>::value, "Not derived from pathos::ShaderStage");
			program.addShaderStage(new Stage1);
			program.addShaderStage(new Stage2);
			program.addShaderStage(new Stage3);
		}

		// VS, TES, TCS, PS
		template<typename Stage1, typename Stage2, typename Stage3, typename Stage4>
		void addStage() {
			static_assert(std::is_base_of<ShaderStage, Stage1>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage2>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage3>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage4>::value, "Not derived from pathos::ShaderStage");
			program.addShaderStage(new Stage1);
			program.addShaderStage(new Stage2);
			program.addShaderStage(new Stage3);
			program.addShaderStage(new Stage4);
		}

		// VS, GS, TES, TCS, PS
		template<typename Stage1, typename Stage2, typename Stage3, typename Stage4, typename Stage5>
		void addStage() {
			static_assert(std::is_base_of<ShaderStage, Stage1>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage2>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage3>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage4>::value, "Not derived from pathos::ShaderStage");
			static_assert(std::is_base_of<ShaderStage, Stage5>::value, "Not derived from pathos::ShaderStage");
			program.addShaderStage(new Stage1);
			program.addShaderStage(new Stage2);
			program.addShaderStage(new Stage3);
			program.addShaderStage(new Stage4);
			program.addShaderStage(new Stage5);
		}

		ShaderProgram program;
	};

	__forceinline ShaderProgram* findShaderProgram(uint32 programHash) {
		auto it = ShaderProgram::shaderDB.find(programHash);
		CHECK(it != ShaderProgram::shaderDB.end());
		return it->second;
	}

}

#define DEFINE_COMPUTE_PROGRAM(ShaderProgramClass, ComputeStageClass) \
	static pathos::ShaderProgramInitializer<ComputeStageClass> _shaderProgram_##ShaderProgramClass(#ShaderProgramClass);

#define FIND_SHADER_PROGRAM(ShaderProgramClass) \
	pathos::ShaderDB::get().find(#ShaderProgramClass)
