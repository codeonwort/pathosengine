#include "indirect_lighting.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/irradiance_baker.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/render_target.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/light_probe_component.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/log.h"
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<float> cvar_gi_skyLightBoost("r.probegi.skyLightBoost", 1.0f, "Indirect lighting boost (sky)");
	static ConsoleVariable<float> cvar_gi_diffuseBoost("r.probegi.diffuseBoost", 1.0f, "Indirect lighting boost (diffuse)");
	static ConsoleVariable<float> cvar_gi_specularBoost("r.probegi.specularBoost", 1.0f, "Indirect lighting boost (specular)");

	struct IrradianceVolumeInfo {
		vector3 minBounds;
		uint32 firstTileID;
		vector3 maxBounds;
		uint32 numProbes;
		vector3ui gridSize;
		uint32 _pad0;
	};
	struct ReflectionProbeInfo {
		vector3 positionWS;
		float captureRadius;
	};
	
	static constexpr uint32 SSBO_0_BINDING_SLOT = 2; // Irradiance volumes
	static constexpr uint32 SSBO_1_BINDING_SLOT = 3; // Reflection probes

	struct UBO_IndirectLighting {
		static const uint32 BINDING_SLOT = 1;

		float skyLightBoost;
		float diffuseBoost;
		float specularBoost;
		float _pad0;

		float skyRadianceProbeMaxLOD;
		float localReflectionProbeMaxLOD;
		uint32 numReflectionProbes;
		uint32 numIrradianceVolumes;

		float irradianceAtlasWidth;
		float irradianceAtlasHeight;
		uint32 irradianceTileCountX;
		uint32 irradianceTileSize;
	};

	class IndirectLightingVS : public ShaderStage {
	public:
		IndirectLightingVS() : ShaderStage(GL_VERTEX_SHADER, "IndirectLightingVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};
	class IndirectLightingFS : public ShaderStage {
	public:
		IndirectLightingFS() : ShaderStage(GL_FRAGMENT_SHADER, "IndirectLightingFS") {
			setFilepath("indirect_lighting.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_IndirectLighting, IndirectLightingVS, IndirectLightingFS);

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

		uint32 maxAtlasTiles = pathos::irradianceProbeTileCountX * pathos::irradianceProbeTileCountY;
		ssbo0.init(maxAtlasTiles * sizeof(IrradianceVolumeInfo), "SSBO_0_IndirectLighting");

		uint32 maxReflectionProbes = pathos::reflectionProbeMaxCount;
		ssbo1.init(maxReflectionProbes * sizeof(ReflectionProbeInfo), "SSBO_1_IndirectLighting");
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

		//////////////////////////////////////////////////////////////////////////
		// Prepare for UBO & SSBO data

		std::vector<IrradianceVolumeInfo> irradianceVolumeInfo;
		for (const IrradianceVolumeProxy* volume : scene->proxyList_irradianceVolume) {
			IrradianceVolumeInfo info;
			info.minBounds = volume->minBounds;
			info.firstTileID = volume->irradianceTileFirstID;
			info.maxBounds = volume->maxBounds;
			info.numProbes = volume->numProbes;
			info.gridSize = volume->gridSize;
			irradianceVolumeInfo.emplace_back(info);
		}

		// #todo-light-probe: Only copy the cubemaps that need to be updated.
		// Copy local cubemaps to the cubemap array.
		std::vector<ReflectionProbeInfo> reflectionProbeInfoArray;
		reflectionProbeInfoArray.reserve(scene->proxyList_reflectionProbe.size());
		{
			GLuint cubemapArray = sceneContext.localSpecularIBLs;
			int32 numReflectionProbes = (int32)scene->proxyList_reflectionProbe.size();
			int32 cubemapIndex = 0;
			for (int32 i = 0; i < numReflectionProbes; ++i)
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

				ReflectionProbeInfo info{ proxy->positionWS, proxy->captureRadius };
				reflectionProbeInfoArray.emplace_back(info);
			}
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
		}

		UBO_IndirectLighting uboData{};
		uboData.skyLightBoost = std::max(0.0f, cvar_gi_skyLightBoost.getFloat());
		uboData.diffuseBoost = std::max(0.0f, cvar_gi_diffuseBoost.getFloat());
		uboData.specularBoost = std::max(0.0f, cvar_gi_specularBoost.getFloat());

		uboData.skyRadianceProbeMaxLOD = badger::max(0.0f, (float)(scene->skyPrefilterEnvMapMipLevels - 1));
		uboData.localReflectionProbeMaxLOD = badger::max(0.0f, (float)(pathos::reflectionProbeNumMips - 1));
		uboData.numReflectionProbes = (uint32)reflectionProbeInfoArray.size();
		uboData.numIrradianceVolumes = (uint32)irradianceVolumeInfo.size();

		uboData.irradianceAtlasWidth = scene->irradianceAtlasWidth;
		uboData.irradianceAtlasHeight = scene->irradianceAtlasHeight;
		uboData.irradianceTileCountX = scene->irradianceTileCountX;
		uboData.irradianceTileSize = scene->irradianceTileSize;

		ubo.update(cmdList, UBO_IndirectLighting::BINDING_SLOT, &uboData);

		if (irradianceVolumeInfo.size() > 0) {
			ssbo0.update(
				cmdList,
				SSBO_0_BINDING_SLOT,
				irradianceVolumeInfo.data(),
				irradianceVolumeInfo.size() * sizeof(IrradianceVolumeInfo));
		}
		if (reflectionProbeInfoArray.size() > 0) {
			ssbo1.update(
				cmdList,
				SSBO_1_BINDING_SLOT,
				reflectionProbeInfoArray.data(),
				reflectionProbeInfoArray.size() * sizeof(ReflectionProbeInfo));
		}

		GLuint* gbuffer_textures = (GLuint*)cmdList.allocateSingleFrameMemory(3 * sizeof(GLuint));
		gbuffer_textures[0] = sceneContext.gbufferA;
		gbuffer_textures[1] = sceneContext.gbufferB;
		gbuffer_textures[2] = sceneContext.gbufferC;

		if (scene->depthProbeAtlas != 0) {
			cmdList.textureParameteri(scene->depthProbeAtlas, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			cmdList.textureParameteri(scene->depthProbeAtlas, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(3, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(4, scene->skyIrradianceMap);
		cmdList.bindTextureUnit(5, scene->skyPrefilterEnvMap);
		cmdList.bindTextureUnit(6, IrradianceBaker::getBRDFIntegrationMap_512());
		cmdList.bindTextureUnit(7, sceneContext.localSpecularIBLs);
		cmdList.bindTextureUnit(8, scene->irradianceAtlas);
		cmdList.bindTextureUnit(9, scene->depthProbeAtlas);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		// Fix a strange bug that IBL maps are randomly persistent across worlds.
		cmdList.bindTextures(0, 10, nullptr);

		// Restore render states
		{
			cmdList.disable(GL_BLEND);

			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

}
