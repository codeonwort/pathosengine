#pragma once

#include "renderpass_base.h"

namespace pathos {

	class AlphaOnlyTexturePass : public MeshRenderPass {

	private:
		static constexpr unsigned int TEXTURE_UNIT = 0;

	protected:
		virtual void createProgram() override;
		GLuint positionLocation, uvLocation;

	public:
		AlphaOnlyTexturePass();
		AlphaOnlyTexturePass(const AlphaOnlyTexturePass& other) = delete;
		AlphaOnlyTexturePass(AlphaOnlyTexturePass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	};

}