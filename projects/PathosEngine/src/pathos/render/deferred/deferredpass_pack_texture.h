#pragma once

#include "deferredpass_pack_base.h"

namespace pathos {

	class MeshDeferredRenderPass_Pack_FlatTexture : public MeshDeferredRenderPass_Pack {

	public:
		MeshDeferredRenderPass_Pack_FlatTexture();

		virtual void render(RenderCommandList& cmdList, Scene* inScene, Camera* inCamera, MeshGeometry* inMesh, Material* inMaterial) override;

	protected:
		virtual void createProgram() override;

		UniformBuffer ubo;

	};

}
