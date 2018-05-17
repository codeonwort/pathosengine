#pragma once

#include "renderpass_base.h"

namespace pathos {

	class WireframePass : public MeshRenderPass {

	protected:
		virtual void createProgram() override;
		GLint uniform_mvp, uniform_color;

	public:
		WireframePass();
		WireframePass(const WireframePass& other) = delete;
		WireframePass(WireframePass&& other) = delete;

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	};

}