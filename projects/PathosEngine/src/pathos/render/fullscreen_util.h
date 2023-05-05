#pragma once

#include "pathos/rhi/render_command_list.h"
#include "badger/types/int_types.h"

namespace pathos {

	enum class EClearTextureFormat : uint32 {
		RGBA16f,
	};

	enum class EClearTextureDimension : uint32 {
		Texture2D,
		TextureCube,
	};

	// Use glClearTexImage() if possible.
	// This util is really written just to clear rgba16f textures.
	void clearTexture2D(
		RenderCommandList& cmdList,
		GLuint texture,
		uint32 width,
		uint32 height,
		EClearTextureFormat format,
		float* clearValues,
		bool bMemoryBarrier = true);

	void clearTextureCube(
		RenderCommandList& cmdList,
		GLuint texture,
		uint32 startMipSize,
		uint32 startMipLevel,
		uint32 mipCount,
		EClearTextureFormat format,
		float* clearValues,
		bool bMemoryBarrier = true);

}
