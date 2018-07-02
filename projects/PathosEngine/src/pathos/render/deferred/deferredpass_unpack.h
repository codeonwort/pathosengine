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
		static constexpr unsigned int MAX_DIRECTIONAL_LIGHTS = 8;
		static constexpr unsigned int MAX_POINT_LIGHTS = 24;

		GLuint debug_godRayTexture();

	public:
		MeshDeferredRenderPass_Unpack(GLuint gbuffer_tex0, GLuint gbuffer_tex1, GLuint gbuffer_tex2, unsigned int width, unsigned int height);
		virtual ~MeshDeferredRenderPass_Unpack();

		//inline void setShadowMapping(ShadowMap* shadow) { shadowMapping = shadow; }
		//inline void setOmnidirectionalShadow(OmnidirectionalShadow* shadow) { omniShadow = shadow; }

		void render(Scene*, Camera*); // plain LDR rendering
		void renderHDR(Scene*, Camera*); // HDR rendering

		void bindFramebuffer(bool hdr); // call this before render() or renderHDR()
		void setDrawBuffers(bool both); // enable only first buffer or both buffers

	protected:
		GLuint program = 0; // shader program (original LDR rendering)
		GLuint program_hdr = 0, program_tone_mapping = 0; // HDR rendering
		GLuint program_blur = 0; // gaussian blur
		GLuint gbuffer_tex0, gbuffer_tex1, gbuffer_tex2; // packed input

		// program
		GLint uniform_ldr_eyeDirection;
		GLint uniform_ldr_numDirLights;
		GLint uniform_ldr_dirLightDirs;
		GLint uniform_ldr_dirLightColors;
		GLint uniform_ldr_numPointLights;
		GLint uniform_ldr_pointLightPos;
		GLint uniform_ldr_pointLightColors;

		// program_hdr
		GLint uniform_hdr_eyeDirection;
		GLint uniform_hdr_eyePosition;
		GLint uniform_hdr_numDirLights;
		GLint uniform_hdr_dirLightDirs;
		GLint uniform_hdr_dirLightColors;
		GLint uniform_hdr_numPointLights;
		GLint uniform_hdr_pointLightPos;
		GLint uniform_hdr_pointLightColors;

		// program_blur
		GLint uniform_blur_horizontal;

		static constexpr unsigned int NUM_HDR_ATTACHMENTS = 2;
		GLuint fbo_hdr, fbo_hdr_attachment[NUM_HDR_ATTACHMENTS];
		GLuint fbo_blur, fbo_blur_attachment;
		GLuint fbo_tone, fbo_tone_attachment;

		class GodRay* godRay = nullptr;
		class DepthOfField* dof = nullptr;

		PlaneGeometry* quad = nullptr;

		void createProgram();
		void createProgram_LDR();
		void createProgram_HDR();
		void createResource_HDR(unsigned int width, unsigned int height);

		void uploadDirectionalLightUniform(Scene*, unsigned int maxLights, bool hdr);
		void uploadPointLightUniform(Scene*, unsigned int maxLights, bool hdr);

	};

}