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
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/util/log.h"
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

namespace pathos {

	static ConsoleVariable<float> cvar_gi_intensity("r.probegi.intensity", 1.0f, "Indirect lighting boost coeff");

	// #todo-light-probe: Put this data into SSBO, not UBO.
	struct RadianceProbeUBOData { vector3 positionWS; float captureRadius; };
	struct IrradianceProbeSSBOData { vector3 positionWS; float captureRadius; vector4 uvBounds; };
	
	static constexpr uint32 SSBO_BINDING_SLOT = 2;

	struct UBO_IndirectLighting {
		static const uint32 BINDING_SLOT = 1;

		float skyRadianceProbeMaxLOD;
		float intensity;
		uint32 numRadianceProbes;
		float radianceProbeMaxLOD;

		vector4ui irradianceAtlasParams; // (numValidTiles, ?, ?, ?)

		RadianceProbeUBOData radianceProbes[pathos::radianceProbeMaxCount];
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
		ssbo.init(maxAtlasTiles * sizeof(IrradianceProbeSSBOData), "SSBO_IndirectLighting");
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

		std::vector<IrradianceProbeSSBOData> irradianceAtlasInfo;
		irradianceAtlasInfo.reserve(pathos::irradianceProbeTileCountX * pathos::irradianceProbeTileCountY);
		for (const IrradianceProbeProxy* probe : scene->proxyList_irradianceProbe) {
			if (probe->irradianceTileID != 0xffffffff) {
				IrradianceProbeSSBOData ssboItem;
				ssboItem.positionWS = probe->positionWS;
				ssboItem.captureRadius = probe->captureRadius;
				ssboItem.uvBounds = probe->irradianceTileBounds;
				irradianceAtlasInfo.emplace_back(ssboItem);
			}
		}

		// #todo-light-probe: Only copy the cubemaps that need to be updated.
		// Copy local cubemaps to the cubemap array.
		std::vector<RadianceProbeUBOData> radianceProbeUBOData;
		radianceProbeUBOData.reserve(scene->proxyList_radianceProbe.size());
		{
			GLuint cubemapArray = sceneContext.localSpecularIBLs;
			int32 numRadianceProbes = (int32)scene->proxyList_radianceProbe.size();
			int32 cubemapIndex = 0;
			for (int32 i = 0; i < numRadianceProbes; ++i)
			{
				RadianceProbeProxy* proxy = scene->proxyList_radianceProbe[i];
				if (proxy->specularIBL == nullptr) {
					continue;
				}

				GLuint cubemap = proxy->specularIBL->getGLTexture();
				GLuint size = radianceProbeCubemapSize;
				for (int32 mip = 0; mip < (int32)pathos::radianceProbeNumMips; ++mip) {
					cmdList.copyImageSubData(
						cubemap, GL_TEXTURE_CUBE_MAP, mip, 0, 0, 0,
						cubemapArray, GL_TEXTURE_CUBE_MAP_ARRAY, mip, 0, 0, cubemapIndex * 6,
						size, size, 6);
					size /= 2;
				}
				++cubemapIndex;

				RadianceProbeUBOData uboData{ proxy->positionWS, proxy->captureRadius };
				radianceProbeUBOData.emplace_back(uboData);
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
		uboData.skyRadianceProbeMaxLOD = badger::max(0.0f, (float)(scene->skyPrefilterEnvMapMipLevels - 1));
		uboData.intensity = badger::max(0.0f, cvar_gi_intensity.getFloat());
		uboData.numRadianceProbes = (uint32)radianceProbeUBOData.size();
		uboData.radianceProbeMaxLOD = badger::max(0.0f, (float)(pathos::radianceProbeNumMips - 1));
		uboData.irradianceAtlasParams.x = (uint32)irradianceAtlasInfo.size();
		for (size_t i = 0; i < radianceProbeUBOData.size(); ++i)
		{
			uboData.radianceProbes[i] = radianceProbeUBOData[i];
		}
		ubo.update(cmdList, UBO_IndirectLighting::BINDING_SLOT, &uboData);

		if (irradianceAtlasInfo.size() > 0) {
			ssbo.update(cmdList, SSBO_BINDING_SLOT, irradianceAtlasInfo.data(), irradianceAtlasInfo.size() * sizeof(IrradianceProbeSSBOData));
		}

		GLuint* gbuffer_textures = (GLuint*)cmdList.allocateSingleFrameMemory(3 * sizeof(GLuint));
		gbuffer_textures[0] = sceneContext.gbufferA;
		gbuffer_textures[1] = sceneContext.gbufferB;
		gbuffer_textures[2] = sceneContext.gbufferC;

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(3, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(4, scene->skyIrradianceMap);
		cmdList.bindTextureUnit(5, scene->skyPrefilterEnvMap);
		cmdList.bindTextureUnit(6, IrradianceBaker::getBRDFIntegrationMap_512());
		cmdList.bindTextureUnit(7, sceneContext.localSpecularIBLs);
		cmdList.bindTextureUnit(8, scene->irradianceAtlas);

		fullscreenQuad->activate_position_uv(cmdList);
		fullscreenQuad->activateIndexBuffer(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		// Fix a strange bug that IBL maps are randomly persistent across worlds.
		cmdList.bindTextures(0, 7, nullptr);

		// Restore render states
		{
			cmdList.disable(GL_BLEND);

			cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

}
