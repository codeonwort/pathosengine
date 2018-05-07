#pragma once

#include "deferredpass_pack_base.h"

namespace pathos {

	class MeshDeferredRenderPass_Pack_SolidColor : public MeshDeferredRenderPass_Pack {

	public:
		MeshDeferredRenderPass_Pack_SolidColor();
		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	protected:
		virtual void createProgram() override;

		GLint uniform_mvTransform;
		GLint uniform_mvpTransform;
		GLint uniform_diffuseColor;

		GLint positionLocation = -1;
		GLint normalLocation = -1;

	};

}