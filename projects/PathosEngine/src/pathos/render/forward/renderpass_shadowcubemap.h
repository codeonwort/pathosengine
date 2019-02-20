#pragma once

#include "renderpass_base.h"

namespace pathos {

	// For visualization of depth texture in shadow mapping
	class ShadowCubeTexturePass : public MeshRenderPass {

	private:
		static constexpr unsigned int TEXTURE_UNIT = 0;

	protected:
		virtual void createProgram() override;

	public:
		ShadowCubeTexturePass();
		ShadowCubeTexturePass(const ShadowCubeTexturePass& other) = delete;
		ShadowCubeTexturePass(ShadowCubeTexturePass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	private:
		GLint uniform_mvpTransform;
		GLint uniform_face;
		GLint uniform_zNear;
		GLint uniform_zFar;

	};

}