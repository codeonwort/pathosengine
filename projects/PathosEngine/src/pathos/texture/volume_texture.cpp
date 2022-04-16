#include "volume_texture.h"

#include <FreeImage.h>

namespace pathos {

	VolumeTexture::~VolumeTexture()
	{
		// #todo-texture: What if the texture is contained in a command list which is not finalized yet?
		if (texture != 0) {
			gRenderDevice->deleteTextures(1, &texture);
		}
		if (bitmapInfo != nullptr) {
			FreeImage_Unload(bitmapInfo);
		}
	}

	void VolumeTexture::setImageData(FIBITMAP* inData)
	{
		bitmapInfo = inData;

		imageWidth = (uint32)FreeImage_GetWidth(bitmapInfo);
		imageHeight = (uint32)FreeImage_GetHeight(bitmapInfo);
		bpp = (uint32)FreeImage_GetBPP(bitmapInfo);
	}

	void VolumeTexture::setDebugName(const char* inDebugName)
	{
		debugName = inDebugName;

		if (debugName != nullptr && texture != 0) {
			gRenderDevice->objectLabel(GL_TEXTURE, texture, -1, debugName);
		}
	}

	void VolumeTexture::initGLResource(uint32 textureWidth, uint32 textureHeight, uint32 textureDepth)
	{
		constexpr bool generateMipmaps = true;

		if (bitmapInfo == nullptr) {
			return;
		}

		uint8* rawBytes = FreeImage_GetBits(bitmapInfo);

		GLenum internalFormat = GL_RGBA8;
		GLenum pixelFormat = GL_RGBA;
		switch (bpp) {
		case 8:  internalFormat = GL_R8;    pixelFormat = GL_RED;  break;
		case 16: internalFormat = GL_RG8;   pixelFormat = GL_RG;   break;
		case 24: internalFormat = GL_RGB8;  pixelFormat = GL_RGB;  break;
		case 32: internalFormat = GL_RGBA8; pixelFormat = GL_RGBA; break;
		default: CHECK_NO_ENTRY();
		}

		uint32 numLODs = 1;
		if (generateMipmaps) {
			numLODs = static_cast<uint32>(floor(log2(std::max(std::max(textureWidth, textureHeight), textureDepth))) + 1);
		}

		auto This = this;
		ENQUEUE_RENDER_COMMAND(
			[This, generateMipmaps, numLODs, internalFormat, textureWidth, textureHeight, textureDepth, pixelFormat, rawBytes](RenderCommandList& cmdList) {
				if (This->texture != 0) {
					gRenderDevice->deleteTextures(1, &This->texture);
					This->texture = 0;
				}
				gRenderDevice->createTextures(GL_TEXTURE_3D, 1, &This->texture);

				const GLuint texture = This->texture;
				const char* debugName = This->debugName;

				cmdList.textureStorage3D(texture, numLODs, internalFormat, textureWidth, textureHeight, textureDepth);
				cmdList.textureSubImage3D(
					texture,
					0,                                         // LOD
					0, 0, 0,                                   // offset
					textureWidth, textureHeight, textureDepth, // size
					pixelFormat, GL_UNSIGNED_BYTE, rawBytes);  // pixels

				if (generateMipmaps) {
					cmdList.generateTextureMipmap(texture);
				}

				cmdList.textureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				cmdList.textureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				cmdList.textureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
				cmdList.textureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
				cmdList.textureParameteri(texture, GL_TEXTURE_WRAP_R, GL_REPEAT);

				if (debugName != nullptr) {
					cmdList.objectLabel(GL_TEXTURE, texture, -1, debugName);
				}
			}
		);
		TEMP_FLUSH_RENDER_COMMAND();
	}

}
