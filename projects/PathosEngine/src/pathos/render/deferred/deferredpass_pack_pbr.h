#pragma once

#include "deferredpass_pack_base.h"

namespace pathos {

	class MeshDeferredRenderPass_Pack_PBR : public MeshDeferredRenderPass_Pack {

	public:
		MeshDeferredRenderPass_Pack_PBR();
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