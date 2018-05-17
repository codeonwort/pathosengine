#pragma once

#include "GL/gl_core_4_3.h"
#include <FreeImage.h>

namespace pathos {

	FIBITMAP* loadImage(const char* filename);
	GLuint loadTexture(FIBITMAP* dib, bool generateMipmap = true);
	GLuint loadCubemapTexture(FIBITMAP* dib[], bool generateMipmap = true);

}