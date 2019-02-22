#pragma once

// #todo: persistent mapped buffer
// https://www.bfilipek.com/2015/01/persistent-mapped-buffers-in-opengl.html

#include "gl_core.h"

template<typename T>
class UniformBuffer {

public:
	UniformBuffer();
	~UniformBuffer();

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer& operator=(const UniformBuffer&) = delete;

	void update(GLuint bindingIndex, const T& data);

private:
	GLuint ubo;

};

//////////////////////////////////////////////////////////////////////////

template<typename T>
UniformBuffer<T>::UniformBuffer()
{
	uint32_t bufferSize = (sizeof(T) + 255) & ~255;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, bufferSize, (GLvoid*)0, GL_DYNAMIC_STORAGE_BIT);
}

template<typename T>
UniformBuffer<T>::~UniformBuffer()
{
	glDeleteBuffers(1, &ubo);
}

template<typename T>
void UniformBuffer<T>::update(GLuint bindingIndex, const T& data)
{
	glNamedBufferSubData(ubo, 0, sizeof(T), &data);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ubo);
}
