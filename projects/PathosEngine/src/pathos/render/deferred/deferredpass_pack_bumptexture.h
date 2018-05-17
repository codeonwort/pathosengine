#pragma once

#include "deferredpass_pack_base.h"

namespace pathos {

	// Bump mapping implementation by simple normal mapping
	class MeshDeferredRenderPass_Pack_BumpTexture : public MeshDeferredRenderPass_Pack {

	private:
		//static constexpr unsigned int SHADOW_MAPPING_TEXTURE_UNIT_START = 4;
		//static constexpr unsigned int OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START = 12;

	public:
		MeshDeferredRenderPass_Pack_BumpTexture();

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	protected:
		virtual void createProgram() override;

		GLint uniform_mvTransform3x3;
		GLint uniform_mvTransform;
		GLint uniform_mvpTransform;
		GLint uniform_lightDirection; // in view space
		GLint uniform_eyeDirection; // in view space

	};

}