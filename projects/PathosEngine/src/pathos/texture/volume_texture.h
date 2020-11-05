#pragma once

#include "gl_core.h"
#include "badger/types/noncopyable.h"

#include "pathos/render/render_device.h"

// #todo-texture: Abstract FreeImage type
struct FIBITMAP;

namespace pathos {

	class VolumeTexture final : public Noncopyable {

	public:
		VolumeTexture()
			: texture(0)
			, debugName(nullptr)
			, bitmapInfo(nullptr)
			, imageWidth(0)
			, imageHeight(0)
			, bpp(0)
		{
		}

		~VolumeTexture();

		void setImageData(FIBITMAP* inData);
		void setDebugName(const char* inDebugName);

		void initGLResource(uint32 textureWidth, uint32 textureHeight, uint32 textureDepth);

		inline GLuint getGLName() const { return texture; }
		inline bool isValid() const { return texture != 0; }

		inline uint32 getSourceImageWidth() const { return imageWidth; }
		inline uint32 getSourceImageHeight() const { return imageHeight; }

	private:
		GLuint texture;
		const char* debugName;

		FIBITMAP* bitmapInfo;
		uint32 imageWidth;
		uint32 imageHeight;
		uint32 bpp;

	};

}
