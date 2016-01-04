#include <pathos/loader/imageloader.h>
#include <iostream>

namespace pathos {

	FIBITMAP* loadImage(const char* filename) {
		FIBITMAP *img = NULL, *dib = NULL;
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename);
		img = FreeImage_Load(fif, filename, 0);
		if (!img) {
			std::cerr << "Error loading: " << filename << std::endl;
		}
		unsigned int bpp = FreeImage_GetBPP(img);
		if (bpp == 32) {
			dib = FreeImage_ConvertTo32Bits(img);
		}else if (bpp = 24) {
			dib = FreeImage_ConvertTo24Bits(img);
		}else {
			std::cerr << "loadImage(): An image with unexpected BPP " << bpp << std::endl;
		}
		FreeImage_Unload(img);
		return dib;
	}

	GLuint loadTexture(FIBITMAP* dib) {
		int w, h;
		unsigned char* data;
		GLuint tex_id = 0;

		data = FreeImage_GetBits(dib);
		w = FreeImage_GetWidth(dib);
		h = FreeImage_GetHeight(dib);
		std::cout << "texture load w: " << w << ", h: " << h << std::endl;

		glGenTextures(1, &tex_id);
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		unsigned int bpp = FreeImage_GetBPP(dib);
		if (bpp == 32) {
			glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
		}else if (bpp == 24) {
			glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		}else {
			std::cerr << "loadTexture(): An image with unexpected BPP " << bpp << std::endl;
		}

		//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);

		return tex_id;
	}

}