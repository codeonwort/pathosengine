#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/material/material.h"
#include "pathos/render/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class MeshDeferredRenderPass_Unpack {

	public:
		// maximum number of lights this renderer can handle
		static constexpr uint32_t MAX_DIRECTIONAL_LIGHTS = 8;
		static constexpr uint32_t MAX_POINT_LIGHTS = 24;

	public:
		MeshDeferredRenderPass_Unpack();
		~MeshDeferredRenderPass_Unpack();

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void bindFramebuffer(RenderCommandList& cmdList, bool hdr); // call this before render() or renderHDR()
		void renderLDR(RenderCommandList& cmdList, Scene* scene, Camera* camera); // plain LDR rendering
		void renderHDR(RenderCommandList& cmdList, Scene* scene, Camera* camera); // HDR rendering

	protected:
		GLuint program_ldr = 0; // shader program (original LDR rendering)
		GLuint program_hdr = 0; // HDR rendering
		UniformBuffer ubo_unpack;

		GLuint program_blur = 0; // gaussian blur

		GLuint program_tone_mapping = 0;
		UniformBuffer ubo_tone_mapping;

		// program_blur
		GLint uniform_blur_horizontal = 0;

		static constexpr uint32 NUM_HDR_ATTACHMENTS = 2;
		GLuint fbo_hdr;
		GLuint fbo_blur;
		GLuint fbo_tone;

		class GodRay* godRay = nullptr;
		class DepthOfField* dof = nullptr;

		PlaneGeometry* quad = nullptr;

		bool use_hdr;

		void createProgram_LDR();
		void createProgram_HDR();
		void createResource_HDR(RenderCommandList& cmdList);

	private:
		bool destroyed = false;

	};

}
