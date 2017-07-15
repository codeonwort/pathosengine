#include "context.h"

namespace pathos {

	void DeferredContext::flushCommands() {
		for (auto& cmd : queue) {
			cmd.execute();
		}
		queue.clear();
	}

	void DeferredContext::genTextures(GLsizei n, GLuint* textures) {
		//queue.emplace_back();
	}

	void DeferredContext::bindTexture(GLenum target, GLuint texture) {
		//
	}

	void DeferredContext::deleteTextures(GLsizei n, GLuint* textures) {
		//
	}

	void DeferredContext::texStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
		//
	}

	void DeferredContext::texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels) {
		//
	}

}