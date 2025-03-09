#pragma once

#include "pathos/rhi/gl_handles.h"
#include "badger/types/int_types.h"
#include "badger/types/vector_types.h"

namespace pathos {

	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
	// https://www.khronos.org/opengl/wiki/Sampler_Object
	struct SamplerCreateParams {
		GLenum WRAP_S        = GL_REPEAT;                // GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER, GL_MIRROR_CLAMP_TO_EDGE
		GLenum WRAP_T        = GL_REPEAT;                // Same as WRAP_S
		GLenum WRAP_R        = GL_REPEAT;                // Same as WRAP_S
		GLenum MIN_FILTER    = GL_NEAREST_MIPMAP_LINEAR; // GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR
		GLenum MAG_FILTER    = GL_LINEAR;                // GL_NEAREST, GL_LINEAR
		vector4 BORDER_COLOR = vector4(0, 0, 0, 0);      // For wrap = GL_CLAMP_TO_BORDER
		float MIN_LOD        = -1000.0f;
		float MAX_LOD        = 1000.0f;
		float LOD_BIAS       = 0.0f;
		GLenum COMPARE_MODE  = GL_NONE;                  // For depth texture. GL_NONE, GL_COMPARE_REF_TO_TEXTURE
		GLenum COMPARE_FUNC  = GL_ALWAYS;                // For depth texture with compare mode = ref to texture. GL_LEQUAL, GL_GEQUAL, GL_LESS, GL_GREATER, GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS, GL_NEVER
	};

	// Wrapper for GPU sampler resource.
	class Sampler final : public Noncopyable {

	public:
		Sampler(const SamplerCreateParams& inCreateParams, const char* inDebugName = nullptr)
			: createParams(inCreateParams)
		{
			debugName = (inDebugName != nullptr) ? inDebugName : "unnamed_sampler";
		}

		~Sampler();

		void createGPUResource(bool flushGPU = false);
		void releaseGPUResource();

		void createGPUResource_renderThread(RenderCommandList& cmdList);
		void releaseGPUResource_renderThread(RenderCommandList& cmdList);

		inline const SamplerCreateParams& getCreateParams() const { return createParams; }
		GLuint internal_getGLName() const;

		// Game logic can assume that this instance have a valid GL sampler.
		inline bool isCreated() const { return bCreated; }

	private:
		const SamplerCreateParams createParams;
		std::string debugName;

		GLuint glSampler = 0;
		bool bCreated = false;

	};

}
