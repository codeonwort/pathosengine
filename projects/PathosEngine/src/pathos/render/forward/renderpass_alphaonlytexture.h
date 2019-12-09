#pragma once

#include "renderpass_base.h"

namespace pathos {

	class AlphaOnlyTexturePass : public MeshRenderPass {

	public:
		AlphaOnlyTexturePass();
		AlphaOnlyTexturePass(const AlphaOnlyTexturePass& other) = delete;
		AlphaOnlyTexturePass(AlphaOnlyTexturePass&& other) = delete;

		virtual void renderMeshPass(RenderCommandList& cmdList, Scene* inScene, Camera* inCamera, MeshGeometry* inMesh, Material* inMaterial) override;

	private:
		virtual void createProgram() override;

		GLint uniform_mvpTransform;
		GLint uniform_texSampler;
		GLint uniform_color;

	};

}
