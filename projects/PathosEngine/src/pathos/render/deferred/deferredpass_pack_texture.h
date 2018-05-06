#pragma once

#include "deferredpass_pack_base.h"

namespace pathos {

	class MeshDeferredRenderPass_Pack_FlatTexture : public MeshDeferredRenderPass_Pack {

	private:
		static constexpr unsigned int TEXTURE_UNIT = 0;

	public:
		MeshDeferredRenderPass_Pack_FlatTexture();
		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	protected:
		virtual void createProgram() override;
		GLint positionLocation = -1, uvLocation = -1, normalLocation = -1;

	};

}