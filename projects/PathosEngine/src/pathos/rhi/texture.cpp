#include "texture.h"

#include "pathos/rhi/render_device.h"
#include "pathos/loader/image_loader.h"
#include "pathos/util/log.h"

namespace pathos {

	Texture::~Texture() {
		releaseGPUResource();
	}

	void Texture::createGPUResource(bool flushGPU /*= false*/) {
		if (created) {
			LOG(LogWarning, "Texture %s was already created", createParams.debugName.size() > 0 ? createParams.debugName : "<noname>");
			return;
		}
		created = true;

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
					if (createParams.imageBlobs.size() > 0) {
						auto blob = createParams.imageBlobs[0];
						cmdList.textureSubImage2D(
							texture,
							0, // LOD
							0, 0, // offset
							createParams.width, createParams.height, // size
							blob->glPixelFormat, blob->glDataType, blob->rawBytes); // pixel data
					}
				} else if (createParams.glDimension == GL_TEXTURE_CUBE_MAP) {
					cmdList.textureStorage2D(
						texture, actualMipLevels, createParams.glStorageFormat,
						createParams.width, createParams.height);
					if (createParams.imageBlobs.size() >= 6) {
						for (int32 face = 0; face < 6; ++face) {
							auto blob = createParams.imageBlobs[face];
							cmdList.textureSubImage3D(
								texture,
								0, // LOD
								0, 0, face, // offset
								createParams.width, createParams.height, 1, // size
								blob->glPixelFormat, blob->glDataType, blob->rawBytes); // pixel data
						}
					}
				} else if (createParams.glDimension == GL_TEXTURE_3D) {
					cmdList.textureStorage3D(
						texture, actualMipLevels, createParams.glStorageFormat,
						createParams.width, createParams.height, createParams.depth);
					if (createParams.imageBlobs.size() > 0) {
						auto blob = createParams.imageBlobs[0];
						cmdList.textureSubImage3D(
							texture,
							0, // LOD
							0, 0, 0, // offset
							createParams.width, createParams.height, createParams.depth, // size
							blob->glPixelFormat, blob->glDataType, blob->rawBytes); // pixel data
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
				if (createParams.autoDestroyImageBlob) {
					for (ImageBlob* blob : createParams.imageBlobs) {
						cmdList.registerDeferredCleanup(blob);
					}
				}
			}
		); // ENQUEUE_RENDER_COMMAND
		if (flushGPU) {
			FLUSH_RENDER_COMMAND(true);
		}
	}

	void Texture::releaseGPUResource() {
		if (glTexture != 0) {
			ENQUEUE_DEFERRED_RENDER_COMMAND([tex = glTexture](RenderCommandList& cmdList) {
				cmdList.registerDeferredTextureCleanup(tex);
			});
			glTexture = 0;
		}
	}

}
