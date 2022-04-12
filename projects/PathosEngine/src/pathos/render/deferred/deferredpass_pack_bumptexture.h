#pragma once

#include "deferredpass_pack_base.h"

namespace pathos {

	// Bump mapping implementation by simple normal mapping
	class MeshDeferredRenderPass_Pack_BumpTexture : public MeshDeferredRenderPass_Pack {

	public:
		MeshDeferredRenderPass_Pack_BumpTexture();

		virtual void render(
			RenderCommandList& cmdList,
			SceneProxy* inScene,
			Camera* inCamera,
			MeshGeometry* inMesh,
			Material* inMaterial) override;

	protected:
		virtual void createProgram() override;

		UniformBuffer ubo;

	};

}
