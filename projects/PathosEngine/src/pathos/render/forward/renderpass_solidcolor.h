#pragma once

#include "renderpass_base.h"

namespace pathos {

	class SolidColorPass : public MeshRenderPass {

	private:
		static constexpr unsigned int SHADOW_MAPPING_TEXTURE_UNIT_START = 4;
		static constexpr unsigned int OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START = 12;

	protected:
		virtual void createProgram() override;

		unsigned int maxDirectionalLights, maxPointLights;

	public:
		SolidColorPass(unsigned int maxDirLights, unsigned int maxPointLights);
		SolidColorPass(const SolidColorPass& other) = delete;
		SolidColorPass(SolidColorPass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, Material*) override;

	};

}