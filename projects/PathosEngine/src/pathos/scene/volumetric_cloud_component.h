#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/rhi/texture.h"
#include "pathos/scene/scene_component.h"
#include "pathos/render/scene_proxy.h"

namespace pathos {

	struct VolumetricCloudProxy : public SceneComponentProxy {
		Texture* weatherTexture;
		Texture* shapeNoise;
		Texture* erosionNoise;
	};

	class VolumetricCloudComponent : public SceneComponent {

	public:
		void setTextures(Texture* inWeatherTexture, Texture* inShapeNoise, Texture* inErosionNoise) {
			weatherTexture = inWeatherTexture;
			shapeNoise = inShapeNoise;
			erosionNoise = inErosionNoise;
		}

		bool hasValidResources() const {
			return weatherTexture != 0
				&& shapeNoise != nullptr && shapeNoise->isCreated()
				&& erosionNoise != nullptr && erosionNoise->isCreated();
		}

	protected:
		virtual void createRenderProxy(SceneProxy* scene) override
		{
			if (!hasValidResources() || scene->cloud != nullptr) {
				return;
			}

			VolumetricCloudProxy* proxy = ALLOC_RENDER_PROXY<VolumetricCloudProxy>(scene);

			proxy->weatherTexture = weatherTexture;
			proxy->shapeNoise = shapeNoise;
			proxy->erosionNoise = erosionNoise;

			scene->cloud = proxy;
		}

	private:
		Texture* weatherTexture = nullptr;
		Texture* shapeNoise = nullptr;
		Texture* erosionNoise = nullptr;

	};

}
