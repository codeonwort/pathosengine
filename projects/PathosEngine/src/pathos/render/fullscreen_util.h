#pragma once

#include "pathos/rhi/render_command_list.h"
#include "badger/types/int_types.h"

namespace pathos {

	/// <summary>
	/// Common vertex shader for fullscreen passes.
	/// </summary>
	/// <typeparam name="ForceZToZero">If true, vertex z values are all 0.0, otherwise 1.0. Default is false. </typeparam>
	template<bool ForceZToZero = false>
	class FullscreenVS : public ShaderStage {
	public:
		FullscreenVS() : ShaderStage(GL_VERTEX_SHADER, "FullscreenVS") {
			if constexpr (ForceZToZero) {
				addDefine("FORCE_Z_TO_ZERO", 1);
			}
			setFilepath("fullscreen_quad.glsl");
		}
	};

	/// Used for clearTexture2D()
	enum class EClearTextureFormat : uint32 {
		RGBA16f,
	};

	/// Used for clearTextureCube()
	enum class EClearTextureDimension : uint32 {
		Texture2D,
		TextureCube,
	};

	/// NOTE: Use glClearTexImage() if possible.
	/// This util was written just to clear rgba16f textures.
	/// AFAIK, OpenGL does not provide a standard way to clear half float textures with glClearTexImage.
	void clearTexture2D(
		RenderCommandList& cmdList,
		GLuint texture,
		uint32 width,
		uint32 height,
		EClearTextureFormat format,
		float* clearValues,
		bool bMemoryBarrier = true);

	/// NOTE: Use glClearTexImage() if possible.
	/// This util was written just to clear rgba16f textures.
	/// AFAIK, OpenGL does not provide a standard way to clear half float textures with glClearTexImage.
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
