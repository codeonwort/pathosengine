#include "texture.h"

#include "pathos/rhi/render_device.h"
#include "pathos/loader/image_loader.h"
#include "pathos/util/log.h"

namespace pathos {

	Texture::~Texture() {
		if (glTexture != 0) {
			GLuint tex = glTexture;
			ENQUEUE_RENDER_COMMAND([tex](RenderCommandList& cmdList) {
				gRenderDevice->deleteTextures(1, &tex);
			});
		}
	}

	void Texture::createGPUResource(bool flushGPU /*= false*/) {
		if (created) {
			LOG(LogWarning, "Texture %s was already created", createParams.debugName.size() > 0 ? createParams.debugName : "<noname>");
			return;
		}

		uint32 actualMipLevels = 1;
		if (createParams.mipLevels != 1) {
			uint32 sizeX = createParams.width;
			uint32 sizeY = createParams.height;
			uint32 sizeZ = createParams.depth;
			if (createParams.glDimension == GL_TEXTURE_1D) sizeY = sizeZ = sizeX;
			if (createParams.glDimension == GL_TEXTURE_2D) sizeZ = sizeY;
			uint32 maxMips = static_cast<uint32>(floor(log2(std::max(std::max(sizeX, sizeY), sizeZ))) + 1);
			actualMipLevels = (createParams.mipLevels == 0) ? maxMips : std::min(createParams.mipLevels, maxMips);
		}

		auto This = this;
		ENQUEUE_RENDER_COMMAND(
			[This, actualMipLevels](RenderCommandList& cmdList) {
				const TextureCreateParams& createParams = This->getCreateParams();
				if (This->glTexture != 0) {
					gRenderDevice->deleteTextures(1, &This->glTexture);
					This->glTexture = 0;
				}
				gRenderDevice->createTextures(createParams.glDimension, 1, &This->glTexture);

				const GLuint texture = This->glTexture;

				if (createParams.glDimension == GL_TEXTURE_2D) {
					cmdList.textureStorage2D(
						texture, actualMipLevels, createParams.glStorageFormat,
						createParams.width, createParams.height);
					if (createParams.imageBlob != nullptr) {
						cmdList.textureSubImage2D(
							texture,
							0, // LOD
							0, 0, // offset
							createParams.width, createParams.height, // size
							createParams.glPixelFormat, createParams.glDataType, // pixel format
							createParams.imageBlob->rawBytes); // pixel data
					}
				} else if (createParams.glDimension == GL_TEXTURE_3D) {
					cmdList.textureStorage3D(
						texture, actualMipLevels, createParams.glStorageFormat,
						createParams.width, createParams.height, createParams.depth);
					if (createParams.imageBlob != nullptr) {
						cmdList.textureSubImage3D(
							texture,
							0, // LOD
							0, 0, 0, // offset
							createParams.width, createParams.height, createParams.depth, // size
							createParams.glPixelFormat, createParams.glDataType, // pixel format
							createParams.imageBlob->rawBytes); // pixel data
					}
				} else {
					CHECKF(0, "WIP: Unhandled glDimension");
				}

				if (actualMipLevels != 1) {
					cmdList.generateTextureMipmap(texture);
				}
				if (createParams.debugName.size() > 0) {
					cmdList.objectLabel(GL_TEXTURE, texture, -1, createParams.debugName.c_str());
				}
				if (createParams.autoDestroyImageBlob && createParams.imageBlob != nullptr) {
					cmdList.registerDeferredCleanup(createParams.imageBlob);
				}
			}
		); // ENQUEUE_RENDER_COMMAND
		if (flushGPU) {
			FLUSH_RENDER_COMMAND(true);
		}
	}

	void Texture::setBitmapData(BitmapBlob* inData) {
		bitmapInfo = inData;
	}

	void Texture::initGLResource(uint32 textureWidth, uint32 textureHeight, uint32 textureDepth) {
		constexpr bool generateMipmaps = true;

		if (bitmapInfo == nullptr) {
			return;
		}

		uint8* rawBytes = bitmapInfo->getRawBytes();
		uint32 bpp = 8;

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
				if (This->glTexture != 0) {
					gRenderDevice->deleteTextures(1, &This->glTexture);
					This->glTexture = 0;
				}
				gRenderDevice->createTextures(GL_TEXTURE_3D, 1, &This->glTexture);

				const GLuint texture = This->glTexture;
				const std::string& debugName = This->getCreateParams().debugName;

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

				if (debugName.size() > 0) {
					cmdList.objectLabel(GL_TEXTURE, texture, -1, debugName.c_str());
				}
			}
		);
		TEMP_FLUSH_RENDER_COMMAND();
	}

}
