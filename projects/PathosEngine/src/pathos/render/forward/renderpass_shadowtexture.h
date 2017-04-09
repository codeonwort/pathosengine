#pragma once

#include "renderpass_base.h"

namespace pathos {

	// For visualization of depth texture in shadow mapping
	class ShadowTexturePass : public MeshRenderPass {

	private:
		static constexpr unsigned int TEXTURE_UNIT = 0;

	protected:
		virtual void createProgram() override;
		GLuint positionLocation, uvLocation;

	public:
		ShadowTexturePass();
		ShadowTexturePass(const ShadowTexturePass& other) = delete;
		ShadowTexturePass(ShadowTexturePass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	};

}