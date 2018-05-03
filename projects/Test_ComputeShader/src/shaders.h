#pragma once

#include "pathos/render/shader.h"

class DepthOfField {

public:
	DepthOfField();
	~DepthOfField();

	void execute(GLuint image);

private:
	void createShaders();
	GLuint createSubsumShader();
	GLuint createBlurShader();

	GLuint program_subsum2D;
	GLuint program_blur;

};