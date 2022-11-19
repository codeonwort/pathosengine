#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/render/render_device.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	struct BitmapBlob;

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

		void setImageData(BitmapBlob* inData);
		void setDebugName(const char* inDebugName);

		void initGLResource(uint32 textureWidth, uint32 textureHeight, uint32 textureDepth);

		inline GLuint getGLName() const { return texture; }
		inline bool isValid() const { return texture != 0; }

		inline uint32 getSourceImageWidth() const { return imageWidth; }
		inline uint32 getSourceImageHeight() const { return imageHeight; }

	private:
		GLuint texture;
		const char* debugName;

		BitmapBlob* bitmapInfo;
		uint32 imageWidth;
		uint32 imageHeight;
		uint32 bpp;

	};

}
