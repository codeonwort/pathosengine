#pragma once

#include "deferredpass_pack_base.h"

namespace pathos {

	struct UBO_Deferred_Pack_PBR {
		glm::mat4 mvMatrix;
		glm::mat4 mvpMatrix;
		glm::mat3 mvMatrix3x3;
	};

	class MeshDeferredRenderPass_Pack_PBR : public MeshDeferredRenderPass_Pack {

	public:
		MeshDeferredRenderPass_Pack_PBR();

		virtual void render(Scene*, Camera*, MeshGeometry*, MeshMaterial*) override;

	protected:
		virtual void createProgram() override;

		UniformBuffer<UBO_Deferred_Pack_PBR> ubo;

	};

}
