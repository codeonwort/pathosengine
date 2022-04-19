// Simulates the Earth's atmosphere scattering

#pragma once

#include "sky.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/actor/scene_component.h"
#include "pathos/shader/uniform_buffer.h"

#include "gl_core.h"

// #todo-refactoring: Separate pass and actor code
namespace pathos {

	class SkyAtmospherePass : public Noncopyable {

	public:
		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		UniformBuffer ubo;
		GLuint vao;

	};

	struct SkyAtmosphereProxy : SceneComponentProxy {
		// No parameters for now
		float dummy = 0.0f;
	};

}

namespace pathos {

	class AtmosphereScatteringComponent : public SceneComponent {
		
	public:
		virtual void createRenderProxy(SceneProxy* scene) override {
			SkyAtmosphereProxy* proxy = ALLOC_RENDER_PROXY<SkyAtmosphereProxy>(scene);
			proxy->dummy = 0.0f;

			scene->skyAtmosphere = proxy;
		}

	};

	class AtmosphereScattering : public SkyActor {
		
	public:
		AtmosphereScattering() {
			component = createDefaultComponent<AtmosphereScatteringComponent>();
			setAsRootComponent(component);
		}

		AtmosphereScatteringComponent* getComponent() const { return component; }

	private:
		AtmosphereScatteringComponent* component;

	};

}
