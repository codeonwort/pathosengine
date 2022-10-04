#pragma once

#include "sky.h"
#include "pathos/gl_handles.h"
#include "pathos/mesh/geometry.h"
#include "pathos/actor/scene_component.h"

#include "badger/types/noncopyable.h"

namespace pathos {

	struct AnselSkyProxy : SceneComponentProxy {
		MeshGeometry* sphere;
		GLuint textureID;
	};

	class AnselSkyPass : public Noncopyable {
		
	public:
		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		GLuint fbo = 0xffffffff;
		GLint uniform_transform = 0xffffffff;

	};

}

namespace pathos {

	// #todo-refactoring: Move to another folder
	class IcosahedronGeometry : public MeshGeometry {

		static constexpr uint32_t INITIAL_NUM_VERTICES = 12;
		static constexpr uint32_t INITIAL_NUM_TRIANLGES = 20;
		
	public:
		IcosahedronGeometry(uint32_t subdivisionStep = 0);

	private:
		void buildGeometry();		// initial positions and indices of icosahedron
		void subdivide();			// subdivision step
		void uploadToGPU();			// GL calls

		GLfloat* positionData;
		GLfloat* uvData;
		GLfloat* normalData;
		GLuint* indexData;

		uint32_t numVertices;
		uint32_t numTriangles;
		
	};

	class AnselSkyComponent : public SceneComponent {

	public:
		~AnselSkyComponent();

		void initialize(GLuint textureID);

		inline bool hasValidResources() const { return textureID != 0 && sphere != nullptr; }

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override;

	private:
		GLuint textureID = 0;
		MeshGeometry* sphere = nullptr;

	};

	class AnselSkyActor : public SkyActor {
		
	public:
		AnselSkyActor() {
			component = createDefaultComponent<AnselSkyComponent>();
			setAsRootComponent(component);
		}

		void initialize(GLuint textureID);

	private:
		AnselSkyComponent* component;

	};

}
