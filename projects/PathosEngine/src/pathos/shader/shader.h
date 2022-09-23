#pragma once

// OpenGL
#include <GL/glcorearb.h>

// STL
#include <string>
#include <vector>
using namespace std;

namespace pathos {

	class Shader;

	// Utilities
	GLuint createProgram(Shader& vs, Shader& fs, const char* debugName);        // only vertex and fragment shaders
	GLuint createProgram(std::vector<Shader*>& shaders, const char* debugName); // multiple shaders

	// Takes shader sources and compile them.
	// sources come from ShaderSource (setSource) or physical files (loadSource).
	class Shader {
	public:
		Shader(GLenum type, const char* debugName = "");
		virtual ~Shader();

		/** Should be called before loadSource() */
		inline void addDefine(const char* define)
		{
			defines.push_back(define);
		}
		/** Should be called before loadSource() */
		inline void addDefine(const std::string& define)
		{
			defines.emplace_back(define);
		}

		void setSource(const std::string& newSource) { source = { newSource }; }

		bool loadSource(const char* filepath);
		bool loadSource(const std::string& filepath);

		bool compile();

		const GLuint getName() { return glName; }
		const char* getErrorLog() { return errorLog.c_str(); }

	protected:
		GLuint glName;
		GLenum type;
		std::vector<std::string> defines;
		std::vector<std::string> source;
		std::string errorLog;
	};

}