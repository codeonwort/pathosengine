#pragma once

// rendering context

#include <GL/glew.h>
#include <vector>

namespace pathos {

	class IContext {

	public:
		virtual void flushCommands() = 0;

		virtual void genTextures(GLsizei n, GLuint* textures) = 0;
		virtual void bindTexture(GLenum target, GLuint texture) = 0;
		virtual void deleteTextures(GLsizei n, GLuint* textures) = 0;
		virtual void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) = 0;
		virtual void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels) = 0;

	};

	// Use it anywhere freely
	class ImmediateContext : public IContext {

	public:
		ImmediateContext() {}
		ImmediateContext(const ImmediateContext& other) = delete;
		ImmediateContext(ImmediateContext&& rhs) = delete;

		void flushCommands() override {} // nothing to do

		void genTextures(GLsizei n, GLuint* textures) override { glGenTextures(n, textures); }
		void bindTexture(GLenum target, GLuint texture) override { glBindTexture(target, texture); }
		void deleteTextures(GLsizei n, GLuint* textures) override { glDeleteTextures(n, textures); }
		void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override { glTexStorage2D(target, levels, internalformat, width, height); }
		void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels) override {
			glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
		}

	};

	struct IDeferredCommand {
		virtual void execute() = 0;
	};
	struct Deffered_genTextures {
		//
	};

	// For worker threads
	class DeferredContext : public IContext {

	public:
		DeferredContext() {}
		DeferredContext(const DeferredContext& other) = delete;
		DeferredContext(DeferredContext&& rhs) = delete;

		void flushCommands() override;

		void genTextures(GLsizei n, GLuint* textures) override;
		void bindTexture(GLenum target, GLuint texture) override;
		void deleteTextures(GLsizei n, GLuint* textures) override;
		void texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) override;
		void texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels) override;

	private:
		std::vector<IDeferredCommand> queue;
		GLuint currentTexture = 0;

	};

}