#pragma once

#include "renderpass_base.h"

namespace pathos {

	// Bump mapping implementation by simple normal mapping
	class BumpTexturePass : public MeshRenderPass {

	private:
		static constexpr unsigned int DIFFUSE_TEXTURE_UNIT = 0;
		static constexpr unsigned int NORMALMAP_TEXTURE_UNIT = 1;
		static constexpr unsigned int SHADOW_MAPPING_TEXTURE_UNIT_START = 4;
		static constexpr unsigned int OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START = 12;

	protected:
		virtual void createProgram() override;

		unsigned int maxDirectionalLights, maxPointLights;

	public:
		BumpTexturePass(unsigned int maxDirLights, unsigned int maxPointLights);
		BumpTexturePass(const BumpTexturePass& other) = delete;
		BumpTexturePass(BumpTexturePass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	};

}