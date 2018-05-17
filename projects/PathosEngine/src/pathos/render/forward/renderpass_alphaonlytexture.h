#pragma once

#include "renderpass_base.h"

namespace pathos {

	class AlphaOnlyTexturePass : public MeshRenderPass {

	public:
		AlphaOnlyTexturePass();
		AlphaOnlyTexturePass(const AlphaOnlyTexturePass& other) = delete;
		AlphaOnlyTexturePass(AlphaOnlyTexturePass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	private:
		virtual void createProgram() override;

		GLint uniform_mvpTransform;
		GLint uniform_texSampler;
		GLint uniform_color;

	};

}