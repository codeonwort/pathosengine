#pragma once

#include "pathos/rhi/gl_handles.h"

#include "badger/types/vector_types.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	/// <summary>
	/// Struct to abstract raw image data.
	/// </summary>
	struct ImageBlob {
		~ImageBlob() {
			if (rawBytes != nullptr) {
				delete[] rawBytes;
			}
		}

		void copyRawBytes(const void* src, uint32 srcWidth, uint32 srcHeight, uint32 srcBpp) {
			width = srcWidth;
			height = srcHeight;
			bpp = srcBpp;
			size_t size = width * height * bpp / 8;
			if (rawBytes) delete[] rawBytes;
			rawBytes = new uint8[size];
			::memcpy_s(rawBytes, size, src, size);
		}

		uint8* rawBytes = nullptr; // reinterpret_cast as needed.
		uint32 width = 0;
		uint32 height = 0;
		uint32 bpp = 0; // bits per pixel

		GLenum glStorageFormat = 0;
		GLenum glPixelFormat = 0;
		GLenum glDataType = 0;

		// Example) A 2D image of size 1920x1080 and of format RGBA32UI.
		//   rawData : uint8*
		//   width   : 1920
		//   height  : 1080
		//   bpp     : 128 (= 16 bytes)
		
		// Example) A 2D image of size 2560x1440 and of format RGBAHalf.
		//   rawData : float*
		//   width   : 2560
		//   height  : 1440
		//   bpp     : 64 (= 8 bytes)
	};

}
