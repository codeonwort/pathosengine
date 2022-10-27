#pragma once

#include "pathos/actor/scene_component.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	struct SkyboxProxy : SceneComponentProxy {
		CubeGeometry* cube;
		GLuint textureID;
		float textureLod;
	};

	class SkyboxComponent : public SceneComponent {

	public:
		~SkyboxComponent();

		void initialize(GLuint inTextureID);
		void setLOD(float inLOD);

		inline bool hasValidResources() const { return cube != nullptr && textureID != 0; }

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		CubeGeometry* cube = nullptr;
		GLuint textureID = 0;
		float lod = 0.0f;

	};

}
