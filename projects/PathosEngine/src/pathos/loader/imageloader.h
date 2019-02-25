#pragma once

#include "gl_core.h"
#include <FreeImage.h>

namespace pathos {

	FIBITMAP* loadImage(const char* filename);
	GLuint loadTexture(FIBITMAP* dib, bool generateMipmap, bool sRGB);
	GLuint loadCubemapTexture(FIBITMAP* dib[], bool generateMipmap = true);

}
