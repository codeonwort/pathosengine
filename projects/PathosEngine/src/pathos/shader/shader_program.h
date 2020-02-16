#pragma once

#include "gl_core.h"

#include <string>
#include <vector>

namespace pathos {

	// Forward decl.
	class ShaderProgram;
	class ShaderStage;

	// Shader program compiled from shader stage sources.
	// #todo-shader-rework: When to initialize GL programs? at demand or on startup, or support both ways?
	class ShaderProgram final {

	public:
		ShaderProgram(const char* inDebugName);
		~ShaderProgram();

		void reload();

		inline bool isValid() const { return programName != 0 && programName != 0xffffffff; }
		inline GLuint getGLName() const { return programName; }

	private:
		const char* debugName;
		GLuint programName;

		std::vector<ShaderStage*> shaderStages;

	};

	// Represents one of VS, GS, TES, TCS, or FS.
	// #todo-shader-rework: Replace pathos::Shader with this.
	class ShaderStage {

	public:
		ShaderStage(GLuint inStageName, const char* inDebugName);
		virtual ~ShaderStage();

		// This can be called multiple times for runtime shader recompilation.
		virtual bool compile() = 0;

		inline GLuint getGLName() const { return stageName; }
		inline const std::string& getShaderSource() const { return shaderSource; }

	private:
		GLuint stageName;
		const char* debugName;
		
		std::string shaderSource;

	};

	// #todo-shader-rework: Code-level shader source assembler. pathos::ShaderSource was my very first attempt for this,
	// but the implementation was really cumbersome, not knowing what am I doing. After working with UE4's vertex factory
	// and material system, I think I can do better now.

}
