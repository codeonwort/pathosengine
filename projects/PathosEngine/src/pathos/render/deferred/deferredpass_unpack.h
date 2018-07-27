#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"

namespace pathos {

	class MeshDeferredRenderPass_Unpack {

	public:
		// maximum number of lights this renderer can handle
		static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 8;
		static constexpr uint32_t MAX_POINT_LIGHTS = 24;

		GLuint debug_godRayTexture();

	public:
		MeshDeferredRenderPass_Unpack(GLuint gbuffer_tex0, GLuint gbuffer_tex1, GLuint gbuffer_tex2, unsigned int width, unsigned int height);
		virtual ~MeshDeferredRenderPass_Unpack();

		//inline void setShadowMapping(ShadowMap* shadow) { shadowMapping = shadow; }
		//inline void setOmnidirectionalShadow(OmnidirectionalShadow* shadow) { omniShadow = shadow; }

		// invoke before render() or renderHDR()
		void setSunDepthMap(GLuint depthMapTexture);

		void render(Scene*, Camera*); // plain LDR rendering
		void renderHDR(Scene*, Camera*); // HDR rendering

		void bindFramebuffer(bool hdr); // call this before render() or renderHDR()
		void setDrawBuffers(bool both); // enable only first buffer or both buffers

	protected:
		GLuint program_ldr = 0; // shader program (original LDR rendering)
		GLuint program_hdr = 0; // HDR rendering
		GLuint program_tone_mapping = 0;
		GLuint program_blur = 0; // gaussian blur
		GLuint gbuffer_tex0, gbuffer_tex1, gbuffer_tex2; // packed input

		// program_tone_mapping
		GLint uniform_tone_mapping_exposure;

		// program_blur
		GLint uniform_blur_horizontal;

		static constexpr unsigned int NUM_HDR_ATTACHMENTS = 2;
		GLuint fbo_hdr, fbo_hdr_attachment[NUM_HDR_ATTACHMENTS];
		GLuint fbo_blur, fbo_blur_attachment;
		GLuint fbo_tone, fbo_tone_attachment;

		GLuint sunDepthMap = 0;

		class GodRay* godRay = nullptr;
		class DepthOfField* dof = nullptr;

		PlaneGeometry* quad = nullptr;

		bool use_hdr;

		void createProgram();
		void createProgram_LDR();
		void createProgram_HDR();
		void createResource_HDR(unsigned int width, unsigned int height);

	};

}