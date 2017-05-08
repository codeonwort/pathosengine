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

		void bindFramebuffer(bool hdr); // call this before render() or renderHDR()
		void setDrawBuffers(bool both); // enable only first buffer or both buffers

	protected:
		GLuint program = 0; // shader program (original LDR rendering)
		GLuint program_hdr = 0, program_tone_mapping = 0; // shader programs for HDR rendering
		GLuint gbuffer_tex0, gbuffer_tex1; // packed input

		static constexpr unsigned int NUM_HDR_ATTACHMENTS = 2;
		GLuint fbo_hdr, fbo_hdr_attachment[NUM_HDR_ATTACHMENTS];

		PlaneGeometry* quad = nullptr;

		void createProgram();
		void createProgram_LDR();
		void createProgram_HDR();
		void createResource_HDR(unsigned int width, unsigned int height);

		void uploadDirectionalLightUniform(Scene*, unsigned int maxLights);
		void uploadPointLightUniform(Scene*, unsigned int maxLights);

	public:
		MeshDeferredRenderPass_Unpack(GLuint gbuffer_tex0, GLuint gbuffer_tex1, unsigned int width, unsigned int height);
		virtual ~MeshDeferredRenderPass_Unpack();

		//inline void setShadowMapping(ShadowMap* shadow) { shadowMapping = shadow; }
		//inline void setOmnidirectionalShadow(OmnidirectionalShadow* shadow) { omniShadow = shadow; }

		void render(Scene*, Camera*); // plain LDR rendering
		void renderHDR(Scene*, Camera*); // HDR rendering

	};

}