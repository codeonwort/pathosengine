#pragma once

#include "pathos/gl_handles.h"
#include "pathos/actor/actor.h"
#include "pathos/actor/scene_component.h"
#include "pathos/render/scene_proxy.h"

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
