#pragma once

#include "gl_core.h"
#include "badger/types/noncopyable.h"

#include "pathos/actor/actor.h"
#include "pathos/actor/scene_component.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class VolumeTexture;

	struct VolumetricCloudProxy : public SceneComponentProxy {
		GLuint         weatherTexture;
		VolumeTexture* shapeNoise;
		VolumeTexture* erosionNoise;
	};

	class VolumetricCloudComponent : public SceneComponent {
		
	public:
		void setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise);

		bool hasValidResources() const;

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override
		{
			if (!hasValidResources()) {
				scene->cloud = nullptr;
				return;
			}

			VolumetricCloudProxy* proxy = ALLOC_RENDER_PROXY<VolumetricCloudProxy>(scene);

			proxy->weatherTexture = weatherTexture;
			proxy->shapeNoise = shapeNoise;
			proxy->erosionNoise = erosionNoise;

			scene->cloud = proxy;
		}

	private:
		GLuint weatherTexture = 0;
		VolumeTexture* shapeNoise = nullptr;
		VolumeTexture* erosionNoise = nullptr;

	};

	// Placeholder for world logic.
	// NOTE: Should be assigned to the Scene::cloud member variable.
	class VolumetricCloudActor : public Actor {

	public:
		VolumetricCloudActor();
		~VolumetricCloudActor() = default;

		void setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise);

		inline VolumetricCloudComponent* getCloudComponent() const { return cloudComponent; }

	private:
		VolumetricCloudComponent* cloudComponent;

	};

}

namespace pathos {

	// Rendering logic used by renderer
	class VolumetricCloudPass : public Noncopyable {

	public:
		~VolumetricCloudPass() = default;

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void renderVolumetricCloud(RenderCommandList& cmdList, SceneProxy* scene);

	private:
		void recreateRenderTarget(RenderCommandList& cmdList, uint32 inWidth, uint32 inHeight, float resolutionScale);

	private:
		uint32 renderTargetWidth = 0;
		uint32 renderTargetHeight = 0;
		UniformBuffer ubo;

	};

}
