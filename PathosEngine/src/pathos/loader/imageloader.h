#pragma once

#include <GL/glew.h>
#include <FreeImage.h>

namespace pathos {

	FIBITMAP* loadImage(const char* filename);
	GLuint loadTexture(FIBITMAP* dib);

}