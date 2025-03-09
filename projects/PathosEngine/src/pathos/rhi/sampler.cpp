#include "sampler.h"

#include "pathos/rhi/render_device.h"
#include "pathos/util/log.h"

namespace pathos {

	Sampler::~Sampler() {
		releaseGPUResource();
	}

	void Sampler::createGPUResource(bool flushGPU /*= false*/) {
		if (bCreated) {
			LOG(LogWarning, "Sampler %s was already created (gl name: %u)", debugName.size() > 0 ? debugName : "<unnamed_sampler>", glSampler);
			return;
		}
		bCreated = true;

		auto This = this;
		ENQUEUE_RENDER_COMMAND([This](RenderCommandList& cmdList) {
			This->createGPUResource_renderThread(cmdList);
		});

		if (flushGPU) {
			FLUSH_RENDER_COMMAND(true);
		}
	}

	void Sampler::releaseGPUResource() {
		if (glSampler != 0) {
			ENQUEUE_DEFERRED_RENDER_COMMAND([This = this](RenderCommandList& cmdList) {
				This->releaseGPUResource_renderThread(cmdList);
			});
		}
	}

	void Sampler::createGPUResource_renderThread(RenderCommandList& cmdList) {
		if (glSampler != 0) {
			gRenderDevice->deleteSamplers(1, &glSampler);
			glSampler = 0;
		}
		gRenderDevice->createSamplers(1, &glSampler);
		bCreated = true;

		cmdList.samplerParameteri(glSampler, GL_TEXTURE_WRAP_S, createParams.WRAP_S);
		cmdList.samplerParameteri(glSampler, GL_TEXTURE_WRAP_T, createParams.WRAP_T);
		cmdList.samplerParameteri(glSampler, GL_TEXTURE_WRAP_R, createParams.WRAP_R);
		cmdList.samplerParameteri(glSampler, GL_TEXTURE_MIN_FILTER, createParams.MIN_FILTER);
		cmdList.samplerParameteri(glSampler, GL_TEXTURE_MAG_FILTER, createParams.MAG_FILTER);
		cmdList.samplerParameterfv(glSampler, GL_TEXTURE_BORDER_COLOR, &createParams.BORDER_COLOR[0]);
		cmdList.samplerParameterf(glSampler, GL_TEXTURE_MIN_LOD, createParams.MIN_LOD);
		cmdList.samplerParameterf(glSampler, GL_TEXTURE_MAX_LOD, createParams.MAX_LOD);
		cmdList.samplerParameterf(glSampler, GL_TEXTURE_LOD_BIAS, createParams.LOD_BIAS);
		cmdList.samplerParameteri(glSampler, GL_TEXTURE_COMPARE_MODE, createParams.COMPARE_MODE);
		cmdList.samplerParameteri(glSampler, GL_TEXTURE_COMPARE_FUNC, createParams.COMPARE_FUNC);

		cmdList.objectLabel(GL_SAMPLER, glSampler, -1, debugName.c_str());
	}

	void Sampler::releaseGPUResource_renderThread(RenderCommandList& cmdList) {
		cmdList.registerDeferredSamplerCleanup(glSampler);
		glSampler = 0;
		bCreated = false;
	}

	GLuint Sampler::internal_getGLName() const {
		CHECK(bCreated && glSampler != 0);
		return glSampler;
	}

}
