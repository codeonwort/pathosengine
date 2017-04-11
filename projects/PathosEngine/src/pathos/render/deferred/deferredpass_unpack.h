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

	protected:
		GLuint program = 0; // shader program name
		GLuint gbuffer_tex0, gbuffer_tex1;

		PlaneGeometry* quad = nullptr;

		virtual void createProgram();
		void uploadDirectionalLightUniform(Scene*, unsigned int maxLights);
		void uploadPointLightUniform(Scene*, unsigned int maxLights);

	public:
		MeshDeferredRenderPass_Unpack(GLuint gbuffer_tex0, GLuint gbuffer_tex1);
		virtual ~MeshDeferredRenderPass_Unpack();

		//inline void setShadowMapping(ShadowMap* shadow) { shadowMapping = shadow; }
		//inline void setOmnidirectionalShadow(OmnidirectionalShadow* shadow) { omniShadow = shadow; }

		virtual void render(Scene*, Camera*);

	};

}