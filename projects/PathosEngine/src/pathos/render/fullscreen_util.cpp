#include "fullscreen_util.h"

#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/gl_debug_group.h"

namespace pathos {

	struct ClearTextureDesc {
		GLenum format;
		uint32 numComponents;
	};

	static ClearTextureDesc toClearDesc(EClearTextureFormat fmt) {
		switch (fmt) {
			case EClearTextureFormat::RGBA16f: return { GL_RGBA16F, 4 };
		}
		CHECK_NO_ENTRY();
		return { GL_RGBA32F, 4 };
	}

	template<EClearTextureFormat clearFormat>
	class ClearTexture2DCS : public ShaderStage {
	public:
		ClearTexture2DCS() : ShaderStage(GL_COMPUTE_SHADER, "ClearTexture2DCS") {
			ClearTextureDesc desc = toClearDesc(clearFormat);
			CHECK(desc.format == GL_RGBA16F);
			addDefine("NUM_COMPONENTS", desc.numComponents);
			addDefine("TEXTURE_DIMENSION", (uint32)EClearTextureDimension::Texture2D);
			setFilepath("clear_texture.glsl");
		}
	};

	template<EClearTextureFormat clearFormat>
	class ClearTextureCubeCS : public ShaderStage {
	public:
		ClearTextureCubeCS() : ShaderStage(GL_COMPUTE_SHADER, "ClearTextureCubeCS") {
			ClearTextureDesc desc = toClearDesc(clearFormat);
			CHECK(desc.format == GL_RGBA16F);
			addDefine("NUM_COMPONENTS", desc.numComponents);
			addDefine("TEXTURE_DIMENSION", (uint32)EClearTextureDimension::TextureCube);
			setFilepath("clear_texture.glsl");
		}
	};

	DEFINE_COMPUTE_PROGRAM(Program_ClearTexture2D_rgba16f, ClearTexture2DCS<EClearTextureFormat::RGBA16f>);
	DEFINE_COMPUTE_PROGRAM(Program_ClearTextureCube_rgba16f, ClearTextureCubeCS<EClearTextureFormat::RGBA16f>);

}

namespace pathos {

	void clearTexture2D(
		RenderCommandList& cmdList,
		GLuint texture,
		uint32 width,
		uint32 height,
		EClearTextureFormat format,
		float* clearValues,
		bool bMemoryBarrier)
	{
		SCOPED_DRAW_EVENT(ClearTexture2D);

		ClearTextureDesc clearDesc = toClearDesc(format);

		GLuint program = 0;
		if (clearDesc.format == GL_RGBA16F) {
			program = FIND_SHADER_PROGRAM(Program_ClearTexture2D_rgba16f).getGLName();
		}
		CHECK(program != 0);
		cmdList.useProgram(program);

		if (clearDesc.numComponents == 4) {
			cmdList.uniform4fv(0, 1, clearValues);
		} else {
			CHECK_NO_ENTRY();
		}
		
		constexpr GLuint slot = 0;
		constexpr GLint mipLevel = 0;
		constexpr GLboolean layered = GL_TRUE;
		constexpr GLint layer = 0;
		cmdList.bindImageTexture(slot, texture, mipLevel, layered, layer, GL_WRITE_ONLY, clearDesc.format);

		GLuint numGroupsX = (width + 7) / 8;
		GLuint numGroupsY = (height + 7) / 8;
		cmdList.dispatchCompute(numGroupsX, numGroupsY, 1);
		if (bMemoryBarrier) {
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
	}

	void clearTextureCube(
		RenderCommandList& cmdList,
		GLuint texture,
		uint32 startMipSize,
		uint32 startMipLevel,
		uint32 mipCount,
		EClearTextureFormat format,
		float* clearValues,
		bool bMemoryBarrier /*= true*/)
	{
		SCOPED_DRAW_EVENT(ClearTextureCube);

		uint32 textureSize = startMipSize;
		ClearTextureDesc clearDesc = toClearDesc(format);

		GLuint program = 0;
		if (clearDesc.format == GL_RGBA16F) {
			program = FIND_SHADER_PROGRAM(Program_ClearTextureCube_rgba16f).getGLName();
		}
		CHECK(program != 0);
		cmdList.useProgram(program);

		if (clearDesc.numComponents == 4) {
			cmdList.uniform4fv(0, 1, clearValues);
		} else {
			CHECK_NO_ENTRY();
		}

		constexpr GLuint slot = 0;
		constexpr GLboolean layered = GL_TRUE;
		constexpr GLint layer = 0;
		for (uint32 mipLevel = startMipLevel; mipLevel < startMipLevel + mipCount; ++mipLevel) {
			cmdList.bindImageTexture(slot, texture, mipLevel, layered, layer, GL_WRITE_ONLY, clearDesc.format);

			GLuint numGroupsX = (textureSize + 7) / 8;
			GLuint numGroupsY = (textureSize + 7) / 8;
			cmdList.dispatchCompute(numGroupsX, numGroupsY, 1);

			textureSize /= 2;
		}
		if (bMemoryBarrier) {
			cmdList.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
	}

}
