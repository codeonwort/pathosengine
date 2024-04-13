#include "gl_live_objects.h"
#include "gl_context_manager.h"
#include "shader_program.h"
#include "pathos/util/log.h"

// Do I have to do this manually?
// https://www.khronos.org/opengl/wiki/Texture_Storage
static uint32 getBytesOfInternalformat(GLint internalformat) {
	switch (internalformat) {
		case GL_RGBA32F: case GL_RGBA32UI: case GL_RGBA32I:
			return 16;
		case GL_RGB32F: case GL_RGB32UI: case GL_RGB32I:
			return 12;
		case GL_RGBA16F: case GL_RG32F: case GL_RGBA16UI:
		case GL_RG32UI: case GL_RGBA16I: case GL_RG32I:
		case GL_RGBA16: case GL_RGBA16_SNORM:
			return 8;
		case GL_RGB16: case GL_RGB16_SNORM: case GL_RGB16F:
		case GL_RGB16UI: case GL_RGB16I:
			return 6;
		case GL_RG16F: case GL_R11F_G11F_B10F: case GL_R32F:
		case GL_RGB10_A2UI: case GL_RGBA8UI: case GL_RG16UI:
		case GL_R32UI: case GL_RGBA8I: case GL_RG16I:
		case GL_R32I: case GL_RGB10_A2: case GL_RGBA8: case GL_RG16:
		case GL_RGBA8_SNORM: case GL_RG16_SNORM: case GL_SRGB8_ALPHA8: case GL_RGB9_E5:
			return 4;
		case GL_RGB8: case GL_RGB8_SNORM: case GL_SRGB8: case GL_RGB8UI: case GL_RGB8I:
			return 3;
		case GL_R16F: case GL_RG8UI: case GL_R16UI: case GL_RG8I:
		case GL_R16I: case GL_RG8: case GL_R16: case GL_RG8_SNORM: case GL_R16_SNORM:
			return 2;
		case GL_R8UI: case GL_R8I: case GL_R8: case GL_R8_SNORM:
			return 1;
	}

	CHECK_NO_ENTRY(); // #todo-rhi: Check compressed formats
	return 0;
}

namespace pathos {

	GLLiveObjects* gGLLiveObjects = nullptr;

	void GLLiveObjects::memreport(int64& outTotalBufferMemory, int64& outTotalTextureMemory) {
		CHECK_GL_CONTEXT_TAKEN();

		outTotalBufferMemory = 0;
		outTotalTextureMemory = 0;
		for (GLuint buffer : aliveGLBuffers) {
			int64 bufferSize = 0;
			glGetNamedBufferParameteri64v(buffer, GL_BUFFER_SIZE, &bufferSize);
			outTotalBufferMemory += bufferSize;
		}
		for (GLuint texture : aliveGLTextures) {
			if (glIsTexture(texture) == false) {
				// How is this a case
				continue;
			}
			GLint baseMip, maxMip;
#if 0
			glGetTextureParameteriv(texture, GL_TEXTURE_BASE_LEVEL, &baseMip);
			glGetTextureParameteriv(texture, GL_TEXTURE_MAX_LEVEL, &maxMip);
#else
			// Querying GL_TEXTURE_MAX_LEVEL always returns 1000 by default :/
			// But I'm not tracking mip levels when calling glTextureStorageXXX().
			// Let's just consider first mip.
			baseMip = maxMip = 0;
#endif
			for (GLint mip = baseMip; mip <= maxMip; ++mip) {
				GLint width, height, depth, internalformat;
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_WIDTH, &width);
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_HEIGHT, &height);
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_DEPTH, &depth);
				glGetTextureLevelParameteriv(texture, mip, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);
				uint32 bytesPerPixel = getBytesOfInternalformat(internalformat);
				int64 mipSize = width * height * depth * bytesPerPixel;
				outTotalTextureMemory += mipSize;
			}
		}
	}

	void GLLiveObjects::reportLiveObjects() {
		LOG(LogDebug, "=== Report Live Objects ===");
		LOG(LogDebug, "Vertex Array       : %u", aliveGLVertexArrays.size());
		LOG(LogDebug, "Texture            : %u", aliveGLTextures.size());
		LOG(LogDebug, "Framebuffer        : %u", aliveGLFramebuffers.size());
		LOG(LogDebug, "Sampler            : %u", aliveGLSamplers.size());
		LOG(LogDebug, "Query              : %u", aliveGLQueries.size());
		LOG(LogDebug, "Transform Feedback : %u", aliveGLTransformFeedbacks.size());
		LOG(LogDebug, "Buffer             : %u", aliveGLBuffers.size());
		LOG(LogDebug, "Renderbuffer       : %u", aliveGLRenderBuffers.size());
		LOG(LogDebug, "Program Pipeline   : %u", aliveGLProgramPipelines.size());
		LOG(LogDebug, "Program            : %u", ShaderDB::get().numPrograms());
		
		size_t totalAlive = 0;
		totalAlive += aliveGLVertexArrays.size();
		totalAlive += aliveGLTextures.size();
		totalAlive += aliveGLFramebuffers.size();
		totalAlive += aliveGLSamplers.size();
		totalAlive += aliveGLQueries.size();
		totalAlive += aliveGLTransformFeedbacks.size();
		totalAlive += aliveGLBuffers.size();
		totalAlive += aliveGLRenderBuffers.size();
		totalAlive += aliveGLProgramPipelines.size();
		totalAlive += ShaderDB::get().numPrograms();
		LOG(LogDebug, "> Total %u GL objects are leaking", totalAlive);
	}

	void GLLiveObjects::genTextures(GLsizei n, GLuint* textures) {
		for (GLsizei i = 0; i < n; ++i) aliveGLTextures.insert(textures[i]);
	}

	void GLLiveObjects::genQueries(GLsizei n, GLuint* queries) {
		for (GLsizei i = 0; i < n; ++i) aliveGLQueries.insert(queries[i]);
	}

	void GLLiveObjects::createVertexArrays(GLsizei n, GLuint* arrays) {
		for (GLsizei i = 0; i < n; ++i) aliveGLVertexArrays.insert(arrays[i]);
	}

	void GLLiveObjects::createTextures(GLenum target, GLsizei n, GLuint* textures) {
		for (GLsizei i = 0; i < n; ++i) aliveGLTextures.insert(textures[i]);
	}

	void GLLiveObjects::createFramebuffers(GLsizei n, GLuint* framebuffers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLFramebuffers.insert(framebuffers[i]);
	}

	void GLLiveObjects::createSamplers(GLsizei n, GLuint* samplers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLSamplers.insert(samplers[i]);
	}

	void GLLiveObjects::createQueries(GLenum target, GLsizei n, GLuint* ids) {
		for (GLsizei i = 0; i < n; ++i) aliveGLQueries.insert(ids[i]);
	}

	void GLLiveObjects::createTransformFeedbacks(GLsizei n, GLuint* ids) {
		for (GLsizei i = 0; i < n; ++i) aliveGLTransformFeedbacks.insert(ids[i]);
	}

	void GLLiveObjects::createBuffers(GLsizei n, GLuint* buffers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLBuffers.insert(buffers[i]);
	}

	void GLLiveObjects::createRenderbuffers(GLsizei n, GLuint* renderbuffers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLRenderBuffers.insert(renderbuffers[i]);
	}

	void GLLiveObjects::createProgramPipelines(GLsizei n, GLuint* pipelines) {
		for (GLsizei i = 0; i < n; ++i) aliveGLProgramPipelines.insert(pipelines[i]);
	}

	void GLLiveObjects::deleteVertexArrays(GLsizei n, const GLuint* arrays) {
		for (GLsizei i = 0; i < n; ++i) aliveGLVertexArrays.erase(arrays[i]);
	}

	void GLLiveObjects::deleteTextures(GLsizei n, const GLuint* textures) {
		for (GLsizei i = 0; i < n; ++i) aliveGLTextures.erase(textures[i]);
	}

	void GLLiveObjects::deleteFramebuffers(GLsizei n, const GLuint* framebuffers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLFramebuffers.erase(framebuffers[i]);
	}

	void GLLiveObjects::deleteSamplers(GLsizei n, const GLuint* samplers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLSamplers.erase(samplers[i]);
	}

	void GLLiveObjects::deleteQueries(GLsizei n, const GLuint* ids) {
		for (GLsizei i = 0; i < n; ++i) aliveGLQueries.erase(ids[i]);
	}

	void GLLiveObjects::deleteTransformFeedbacks(GLsizei n, const GLuint* ids) {
		for (GLsizei i = 0; i < n; ++i) aliveGLTransformFeedbacks.erase(ids[i]);
	}

	void GLLiveObjects::deleteBuffers(GLsizei n, const GLuint* buffers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLBuffers.erase(buffers[i]);
	}

	void GLLiveObjects::deleteRenderbuffers(GLsizei n, const GLuint* renderbuffers) {
		for (GLsizei i = 0; i < n; ++i) aliveGLRenderBuffers.erase(renderbuffers[i]);
	}

	void GLLiveObjects::deleteProgramPipelines(GLsizei n, const GLuint* pipelines) {
		for (GLsizei i = 0; i < n; ++i) aliveGLProgramPipelines.erase(pipelines[i]);
	}

}
