#pragma once

#include "renderpass_base.h"

namespace pathos {

	class CubeEnvMapPass : public MeshRenderPass {

	private:
		static constexpr unsigned int TEXTURE_UNIT = 0;

	protected:
		virtual void createProgram() override;
		GLuint positionLocation, normalLocation;
		GLint uniform_eye = -1, uniform_sampler = -1;

	public:
		CubeEnvMapPass();
		CubeEnvMapPass(const CubeEnvMapPass& other) = delete;
		CubeEnvMapPass(CubeEnvMapPass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	};

}