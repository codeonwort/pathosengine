#pragma once

#include "gl_core.h"
#include "badger/types/noncopyable.h"

#include "pathos/actor/actor.h"
#include "pathos/shader/uniform_buffer.h"

namespace pathos {

	class VolumeTexture;

	// Placeholder for world logic
	// NOTE: Should be assigned to the Scene::cloud member variable.
	class VolumetricCloudActor : public Actor {
		friend class DeferredRenderer;

	public:
		VolumetricCloudActor() = default;
		virtual ~VolumetricCloudActor() = default;

		void setTextures(GLuint inWeatherTexture, VolumeTexture* inShapeNoise, VolumeTexture* inErosionNoise);

		bool hasValidResources() const;

	private:
		GLuint weatherTexture = 0;
		VolumeTexture* shapeNoise = nullptr;
		VolumeTexture* erosionNoise = nullptr;

	};

}

namespace pathos {

	struct VolumetricCloudSettings {
		GLuint weatherTexture;
		GLuint shapeNoiseTexture;
		GLuint erosionNoiseTexture;
		uint32 renderTargetWidth;
		uint32 renderTargetHeight;
		uint32 frameCounter;
	};

	// Rendering logic used by renderer
	class VolumetricCloud : public Noncopyable {

	public:
		~VolumetricCloud() = default;

		void initializeResources(RenderCommandList& cmdList);
		void destroyResources(RenderCommandList& cmdList);

		void render(RenderCommandList& cmdList, const VolumetricCloudSettings& settings);
		void recreateRenderTarget(RenderCommandList& cmdList, uint32 inWidth, uint32 inHeight, float resolutionScale);

	private:
		uint32 renderTargetWidth = 0;
		uint32 renderTargetHeight = 0;
		UniformBuffer ubo;

	};

}