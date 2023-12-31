#pragma once

#include "gl_handles.h"

#include "badger/types/int_types.h"
#include "badger/types/noncopyable.h"
#include <set>

namespace pathos {

	// Tracks alive GL objects that were created by OpenGLDevice or RenderCommandList.
	// Cannot track those created by direct GL calls.
	class GLLiveObjects final : public Noncopyable {

	public:
		void memreport(int64& outTotalBufferMemory, int64& outTotalTextureMemory);
		void reportLiveObjects();

		void genTextures(GLsizei n, GLuint* textures);
		void genQueries(GLsizei n, GLuint* queries);

		void createVertexArrays(GLsizei n, GLuint* arrays);
		void createTextures(GLenum target, GLsizei n, GLuint* textures);
		void createFramebuffers(GLsizei n, GLuint* framebuffers);
		void createSamplers(GLsizei n, GLuint* samplers);
		void createQueries(GLenum target, GLsizei n, GLuint* ids);
		void createTransformFeedbacks(GLsizei n, GLuint* ids);
		void createBuffers(GLsizei n, GLuint* buffers);
		void createRenderbuffers(GLsizei n, GLuint* renderbuffers);
		void createProgramPipelines(GLsizei n, GLuint* pipelines);

		void deleteVertexArrays(GLsizei n, const GLuint* arrays);
		void deleteTextures(GLsizei n, const GLuint* textures);
		void deleteFramebuffers(GLsizei n, const GLuint* framebuffers);
		void deleteSamplers(GLsizei n, const GLuint* samplers);
		void deleteQueries(GLsizei n, const GLuint* ids);
		void deleteTransformFeedbacks(GLsizei n, const GLuint* ids);
		void deleteBuffers(GLsizei n, const GLuint* buffers);
		void deleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
		void deleteProgramPipelines(GLsizei n, const GLuint* pipelines);

	private:
		std::set<GLuint> aliveGLVertexArrays;
		std::set<GLuint> aliveGLTextures;
		std::set<GLuint> aliveGLFramebuffers;
		std::set<GLuint> aliveGLSamplers;
		std::set<GLuint> aliveGLQueries;
		std::set<GLuint> aliveGLTransformFeedbacks;
		std::set<GLuint> aliveGLBuffers;
		std::set<GLuint> aliveGLRenderBuffers;
		std::set<GLuint> aliveGLProgramPipelines;
	};

	extern GLLiveObjects* gGLLiveObjects;

}
