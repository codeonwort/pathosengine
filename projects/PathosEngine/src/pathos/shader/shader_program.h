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

		inline bool isValid() const { return glName != 0 && glName != 0xffffffff; }
		inline GLuint getGLName() const { return glName; }

	private:
		const char* debugName;
		GLuint glName;

		std::vector<ShaderStage*> shaderStages;

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

}
