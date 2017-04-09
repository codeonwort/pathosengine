#pragma once

#include "renderpass_base.h"

namespace pathos {

	class FlatTexturePass : public MeshRenderPass {

	private:
		static constexpr unsigned int DIFFUSE_TEXTURE_UNIT = 0;
		static constexpr unsigned int SHADOW_MAPPING_TEXTURE_UNIT_START = 4;
		static constexpr unsigned int OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START = 12;

	protected:
		virtual void createProgram() override;
		GLuint positionLocation, uvLocation, normalLocation;

		unsigned int maxDirectionalLights, maxPointLights;

	public:
		FlatTexturePass(unsigned int maxDirLights, unsigned int maxPointLights);
		FlatTexturePass(const FlatTexturePass& other) = delete;
		FlatTexturePass(FlatTexturePass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	};

}