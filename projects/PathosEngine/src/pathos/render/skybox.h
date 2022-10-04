#pragma once

#include "sky.h"
#include "pathos/gl_handles.h"
#include "pathos/actor/scene_component.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/shader/uniform_buffer.h"

// #todo-refactoring: Separate pass and actor code
namespace pathos {

	class SkyboxPass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		GLuint fbo = 0xffffffff;
		UniformBuffer ubo;

	};

	struct SkyboxProxy : SceneComponentProxy {
		CubeGeometry* cube;
		GLuint textureID;
		float textureLod;
	};

}

namespace pathos {

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

	class Skybox : public SkyActor {

	public:
		Skybox() {
			component = createDefaultComponent<SkyboxComponent>();
			setAsRootComponent(component);
		}

		void initialize(GLuint textureID);
		void setLOD(float inLOD);

		inline SkyboxComponent* getSkyboxComponent() const { return component; }

	private:
		SkyboxComponent* component;
	};

}
