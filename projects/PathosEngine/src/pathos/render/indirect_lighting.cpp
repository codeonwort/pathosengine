#include "indirect_lighting.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/buffer.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/render_target.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/render/image_based_lighting.h"
#include "pathos/render/light_probe_baker.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/reflection_probe_component.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/log.h"
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

namespace pathos {

	ConsoleVariable<int32> cvar_indirectLighting("r.indirectLighting", 1, "0 = disable, 1 = all, 2 = diffuse only, 3 = sky diffuse only, 4 = specular only");
	static ConsoleVariable<float> cvar_indirectLighting_skyBoost("r.indirectLighting.skyBoost", 1.0f, "Sky indirect diffuse/specular boost");
	static ConsoleVariable<float> cvar_indirectLighting_diffuseBoost("r.indirectLighting.diffuseBoost", 1.0f, "IrradianceVolume indirect diffuse boost");
	static ConsoleVariable<float> cvar_indirectLighting_specularBoost("r.indirectLighting.specularBoost", 1.0f, "ReflectionProbe indirect specular boost");

	static constexpr uint32 SSBO_IrradianceVolume_BINDING_SLOT = 2; // Irradiance volumes
	static constexpr uint32 SSBO_ReflectionProbe_BINDING_SLOT = 3; // Reflection probes
	static constexpr uint32 SSBO_SkyDiffuseSH_BINDING_SLOT = 4;

	struct UBO_IndirectLighting {
		static const uint32 BINDING_SLOT = 1;

		float  skyLightBoost;
		float  diffuseBoost;
		float  specularBoost;
		uint32 lightingMode; // cvar_indirectLighting

		float  skyRadianceProbeMaxLOD;
		float  localReflectionProbeMaxLOD;
		uint32 numReflectionProbes;
		uint32 numIrradianceVolumes;

		float  irradianceAtlasWidth;
		float  irradianceAtlasHeight;
		uint32 irradianceTileCountX;
		uint32 irradianceTileSize;
	};

	class IndirectLightingFS : public ShaderStage {
	public:
		IndirectLightingFS() : ShaderStage(GL_FRAGMENT_SHADER, "IndirectLightingFS") {
			setFilepath("indirect_lighting.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_IndirectLighting, FullscreenVS, IndirectLightingFS);

}

namespace pathos {

	IndirectLightingPass::IndirectLightingPass() {}

	IndirectLightingPass::~IndirectLightingPass() {
		CHECK(destroyed);
	}

	void IndirectLightingPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		ubo.init<UBO_IndirectLighting>("UBO_IndirectLighting");
	}

	void IndirectLightingPass::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		destroyed = true;
	}

	void IndirectLightingPass::renderIndirectLighting(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* fullscreenQuad)
	{
		SCOPED_DRAW_EVENT(IndirectLighting);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		const GLuint irradianceVolumeBuffer = scene->irradianceVolumeBuffer;
		const GLuint reflectionProbeBuffer = scene->reflectionProbeBuffer;

		//////////////////////////////////////////////////////////////////////////
		// Prepare for UBO & SSBO data

		// #todo-light-probe: Only copy the cubemaps that need to be updated.
		// Copy local cubemaps to the cubemap array.
		int32 indirectLightingMode = cvar_indirectLighting.getInt();
		if (indirectLightingMode == 1 || indirectLightingMode == 4) {
			const size_t numReflectionProbes = scene->proxyList_reflectionProbe.size();
			GLuint cubemapArray = sceneContext.localSpecularIBLs;

			int32 cubemapIndex = 0;
			for (size_t i = 0; i < numReflectionProbes; ++i)
			{
				ReflectionProbeProxy* proxy = scene->proxyList_reflectionProbe[i];
				if (proxy->specularIBL == nullptr) {
					continue;
				}

				GLuint cubemap = proxy->specularIBL->getGLTexture();
				GLuint size = reflectionProbeCubemapSize;
				for (int32 mip = 0; mip < (int32)pathos::reflectionProbeNumMips; ++mip) {
					cmdList.copyImageSubData(
						cubemap, GL_TEXTURE_CUBE_MAP, mip, 0, 0, 0,
						cubemapArray, GL_TEXTURE_CUBE_MAP_ARRAY, mip, 0, 0, cubemapIndex * 6,
						size, size, 6);
					size /= 2;
				}
				++cubemapIndex;
			}
		}

		UBO_IndirectLighting uboData;
		{
			uboData.skyLightBoost              = std::max(0.0f, cvar_indirectLighting_skyBoost.getFloat());
			uboData.diffuseBoost               = std::max(0.0f, cvar_indirectLighting_diffuseBoost.getFloat());
			uboData.specularBoost              = std::max(0.0f, cvar_indirectLighting_specularBoost.getFloat());
			uboData.lightingMode               = (uint32)indirectLightingMode;
			uboData.skyRadianceProbeMaxLOD     = badger::max(0.0f, (float)(sceneContext.getSkyPrefilterMapMipCount() - 1));
			uboData.localReflectionProbeMaxLOD = badger::max(0.0f, (float)(pathos::reflectionProbeNumMips - 1));
			uboData.numReflectionProbes        = (uint32)scene->proxyList_reflectionProbe.size();
			uboData.numIrradianceVolumes       = (uint32)scene->proxyList_irradianceVolume.size();
			uboData.irradianceAtlasWidth       = scene->irradianceAtlasWidth;
			uboData.irradianceAtlasHeight      = scene->irradianceAtlasHeight;
			uboData.irradianceTileCountX       = scene->irradianceTileCountX;
			uboData.irradianceTileSize         = scene->irradianceTileSize;
		}

		//////////////////////////////////////////////////////////////////////////

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_IndirectLighting);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		pathos::checkFramebufferStatus(cmdList, fbo, "[IndirectLighting] FBO is invalid");

		// Set render states
		{
			cmdList.disable(GL_DEPTH_TEST);
			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
			cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		}

		ubo.update(cmdList, UBO_IndirectLighting::BINDING_SLOT, &uboData);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_IrradianceVolume_BINDING_SLOT, irradianceVolumeBuffer);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_ReflectionProbe_BINDING_SLOT, reflectionProbeBuffer);
		sceneContext.skyDiffuseSH->bindAsSSBO(cmdList, SSBO_SkyDiffuseSH_BINDING_SLOT);

		GLuint* gbuffer_textures = (GLuint*)cmdList.allocateSingleFrameMemory(3 * sizeof(GLuint));
		gbuffer_textures[0] = sceneContext.gbufferA;
		gbuffer_textures[1] = sceneContext.gbufferB;
		gbuffer_textures[2] = sceneContext.gbufferC;

		if (scene->depthProbeAtlas != 0) {
			cmdList.textureParameteri(scene->depthProbeAtlas, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(scene->depthProbeAtlas, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(3, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(5, sceneContext.getSkyPrefilterMapWithFallback());
		cmdList.bindTextureUnit(6, LightProbeBaker::get().getBRDFIntegrationMap_512());
		cmdList.bindTextureUnit(7, sceneContext.localSpecularIBLs);
		cmdList.bindTextureUnit(8, scene->irradianceAtlas);
		cmdList.bindTextureUnit(9, scene->depthProbeAtlas);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		// Fix a strange bug that IBL maps are randomly persistent across worlds.
		cmdList.bindBuffersBase(GL_SHADER_STORAGE_BUFFER, SSBO_IrradianceVolume_BINDING_SLOT, 3, nullptr);
		cmdList.bindTextures(0, 10, nullptr);

		// Restore render states
		{
			cmdList.disable(GL_BLEND);
			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

}
