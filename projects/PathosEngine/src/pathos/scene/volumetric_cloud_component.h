#pragma once

#include "pathos/rhi/gl_handles.h"
#include "pathos/actor/scene_component.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/rhi/volume_texture.h"

namespace pathos {

	struct VolumetricCloudProxy : public SceneComponentProxy {
		GLuint         weatherTexture;
		VolumeTexture* shapeNoise;
		VolumeTexture* erosionNoise;
	};

	class VolumetricCloudComponent : public SceneComponent {

	public:
		void setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise) {
			weatherTexture = inWeatherTexture;
			shapeNoise = inShapeNoise;
			erosionNoise = inErosionNoise;
		}

		bool hasValidResources() const {
			return weatherTexture != 0
				&& shapeNoise != nullptr && shapeNoise->isValid()
				&& erosionNoise != nullptr && erosionNoise->isValid();
		}

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

}
