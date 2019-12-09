#pragma once

#include "renderpass_base.h"

namespace pathos {

	// For visualization of depth texture in shadow mapping
	class ShadowTexturePass : public MeshRenderPass {

	private:
		static constexpr unsigned int TEXTURE_UNIT = 0;

	protected:
		virtual void createProgram() override;

	public:
		ShadowTexturePass();
		ShadowTexturePass(const ShadowTexturePass& other) = delete;
		ShadowTexturePass(ShadowTexturePass&& other) = delete;

		virtual void renderMeshPass(RenderCommandList& cmdList, Scene* inScene, Camera* inCamera, MeshGeometry* inMesh, Material* inMaterial) override;

	};

}