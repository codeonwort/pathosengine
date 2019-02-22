#pragma once

// #todo: persistent mapped buffer
// https://www.bfilipek.com/2015/01/persistent-mapped-buffers-in-opengl.html

#include "gl_core.h"

class UniformBuffer {

public:
	UniformBuffer() {
		ubo = 0;
	}
	~UniformBuffer() {
		if (ubo != 0) {
			glDeleteBuffers(1, &ubo);
		}
	}

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer& operator=(const UniformBuffer&) = delete;

	template<typename T>
	void init() {
		//bufferSize = (sizeof(T) + 255) & ~255;
		bufferSize = sizeof(T);
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferStorage(GL_UNIFORM_BUFFER, bufferSize, (GLvoid*)0, GL_DYNAMIC_STORAGE_BIT);
	}

	void update(GLuint bindingIndex, void* data) {
		assert(ubo);
		glNamedBufferSubData(ubo, 0, bufferSize, data);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ubo);
	}

private:
	GLuint ubo;
	uint32_t bufferSize;

};
