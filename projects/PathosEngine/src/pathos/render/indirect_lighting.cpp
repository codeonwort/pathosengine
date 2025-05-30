#include "indirect_lighting.h"

#include "pathos/engine.h"
#include "pathos/console.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/buffer.h"
#include "pathos/rhi/texture.h"
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
#include "pathos/util/engine_util.h"

#include "badger/assertion/assertion.h"
#include "badger/math/minmax.h"

namespace pathos {

	ConsoleVariable<int32> cvar_indirectLighting("r.indirectLighting", 1, "0 = disable, 1 = all, 2 = diffuse only, 3 = sky diffuse only, 4 = specular only");
	static ConsoleVariable<float> cvar_indirectLighting_skyBoost("r.indirectLighting.skyBoost", 1.0f, "Sky indirect diffuse/specular boost");
	static ConsoleVariable<float> cvar_indirectLighting_diffuseBoost("r.indirectLighting.diffuseBoost", 1.0f, "IrradianceVolume indirect diffuse boost");
	static ConsoleVariable<float> cvar_indirectLighting_specularBoost("r.indirectLighting.specularBoost", 1.0f, "ReflectionProbe indirect specular boost");

	static bool shouldRenderDiffuseGI() {
		int32 n = cvar_indirectLighting.getInt();
		return n == 1 || n == 2 || n == 3;
	}
	static bool shouldRenderSpecularGI() {
		int32 n = cvar_indirectLighting.getInt();
		return n == 1 || n == 4;
	}

	// For diffuse GI
	static constexpr uint32 SSBO_IrradianceVolume_BINDING_SLOT = 2;
	static constexpr uint32 SSBO_SkyDiffuseSH_BINDING_SLOT = 3;
	static constexpr uint32 SSBO_LightProbeSH_BINDING_SLOT = 4;
	// For specular GI
	static constexpr uint32 SSBO_ReflectionProbe_BINDING_SLOT = 2;

	struct UBO_IndirectDiffuseLighting {
		static const uint32 BINDING_SLOT = 1;

		uint32 lightingMode; // cvar_indirectLighting
		uint32 numIrradianceVolumes;
		uint32 irradianceTileCountX;
		uint32 irradianceTileSize;

		float  irradianceAtlasWidth;
		float  irradianceAtlasHeight;
		float  skyLightBoost;
		float  diffuseBoost;
	};

	struct UBO_IndirectSpecularLighting {
		static const uint32 BINDING_SLOT = 1;

		uint32 lightingMode; // cvar_indirectLighting
		uint32 numReflectionProbes;
		uint32 _pad0;
		uint32 _pad1;

		float  skyLightBoost;
		float  specularBoost;
		float  skyRadianceProbeMaxLOD;
		float  localReflectionProbeMaxLOD;
	};

	class IndirectDiffuseLightingFS : public ShaderStage {
	public:
		IndirectDiffuseLightingFS() : ShaderStage(GL_FRAGMENT_SHADER, "IndirectDiffuseLightingFS") {
			setFilepath("gi/indirect_diffuse_lighting.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_IndirectDiffuseLighting, FullscreenVS, IndirectDiffuseLightingFS);

	class IndirectSpecularLightingFS : public ShaderStage {
	public:
		IndirectSpecularLightingFS() : ShaderStage(GL_FRAGMENT_SHADER, "IndirectSpecularLightingFS") {
			setFilepath("gi/indirect_specular_lighting.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_IndirectSpecularLighting, FullscreenVS, IndirectSpecularLightingFS);

}

namespace pathos {

	IndirectLightingPass::IndirectLightingPass() {}

	IndirectLightingPass::~IndirectLightingPass() {
		CHECK(destroyed);
	}

	void IndirectLightingPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);
		uboDiffuse.init<UBO_IndirectDiffuseLighting>("UBO_IndirectDiffuseLighting");
		uboSpecular.init<UBO_IndirectSpecularLighting>("UBO_IndirectSpecularLighting");
	}

	void IndirectLightingPass::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			gRenderDevice->deleteFramebuffers(1, &fbo);
		}
		destroyed = true;
	}

	void IndirectLightingPass::renderIndirectLighting(RenderCommandList& cmdList, SceneProxy* scene)
	{
		SCOPED_DRAW_EVENT(IndirectLighting);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		pathos::checkFramebufferStatus(cmdList, fbo, "[IndirectLighting] FBO is invalid");

		// Set render states
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.enable(GL_BLEND);
		cmdList.blendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE); // Additive blending to sceneColor
		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		if (shouldRenderDiffuseGI()) {
			renderIndirectDiffuse(cmdList, scene);
		}
		if (shouldRenderSpecularGI()) {
			renderIndirectSpecular(cmdList, scene);
		}

		// Restore render states
		cmdList.disable(GL_BLEND);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, 0, 0);
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void IndirectLightingPass::renderIndirectDiffuse(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(IndirectDiffuseLighting);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		auto fullscreenQuad = gEngine->getSystemGeometryUnitPlane();
		const GLuint irradianceVolumeBuffer = scene->irradianceVolumeBuffer;
		const int32 indirectLightingMode = cvar_indirectLighting.getInt();

		UBO_IndirectDiffuseLighting uboData;
		{
			uboData.lightingMode               = (uint32)indirectLightingMode;
			uboData.numIrradianceVolumes       = (uint32)scene->proxyList_irradianceVolume.size();
			uboData.irradianceTileCountX       = scene->irradianceTileCountX;
			uboData.irradianceTileSize         = scene->irradianceTileSize;
			uboData.irradianceAtlasWidth       = scene->irradianceAtlasWidth;
			uboData.irradianceAtlasHeight      = scene->irradianceAtlasHeight;
			uboData.skyLightBoost              = std::max(0.0f, cvar_indirectLighting_skyBoost.getFloat());
			uboData.diffuseBoost               = std::max(0.0f, cvar_indirectLighting_diffuseBoost.getFloat());
		}

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_IndirectDiffuseLighting);
		cmdList.useProgram(program.getGLName());

		uboDiffuse.update(cmdList, UBO_IndirectDiffuseLighting::BINDING_SLOT, &uboData);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_IrradianceVolume_BINDING_SLOT, irradianceVolumeBuffer);
		sceneContext.skyDiffuseSH->bindAsSSBO(cmdList, SSBO_SkyDiffuseSH_BINDING_SLOT);
		if (scene->irradianceSHBuffer != nullptr) {
			scene->irradianceSHBuffer->bindAsSSBO(cmdList, SSBO_LightProbeSH_BINDING_SLOT);
		} else {
			cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_LightProbeSH_BINDING_SLOT, 0);
		}

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
		cmdList.bindTextureUnit(4, scene->irradianceAtlas);
		cmdList.bindTextureUnit(5, scene->depthProbeAtlas);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		cmdList.bindBuffersBase(GL_SHADER_STORAGE_BUFFER, SSBO_IrradianceVolume_BINDING_SLOT, 4, nullptr);
		cmdList.bindTextures(0, 6, nullptr); // Fix a strange bug that IBL maps are randomly persistent across worlds.
	}

	void IndirectLightingPass::renderIndirectSpecular(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(IndirectSpecularLighting);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		auto fullscreenQuad = gEngine->getSystemGeometryUnitPlane();
		const GLuint reflectionProbeBuffer = scene->reflectionProbeBuffer;

		UBO_IndirectSpecularLighting uboData;
		{
			uboData.lightingMode               = (uint32)cvar_indirectLighting.getInt();
			uboData.numReflectionProbes        = (uint32)scene->proxyList_reflectionProbe.size();
			uboData.skyLightBoost              = std::max(0.0f, cvar_indirectLighting_skyBoost.getFloat());
			uboData.specularBoost              = std::max(0.0f, cvar_indirectLighting_specularBoost.getFloat());
			uboData.skyRadianceProbeMaxLOD     = badger::max(0.0f, (float)(sceneContext.getSkyPrefilterMapMipCount() - 1));
			uboData.localReflectionProbeMaxLOD = badger::max(0.0f, (float)(pathos::reflectionProbeNumMips - 1));
		}

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_IndirectSpecularLighting);
		cmdList.useProgram(program.getGLName());

		uboSpecular.update(cmdList, UBO_IndirectSpecularLighting::BINDING_SLOT, &uboData);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_ReflectionProbe_BINDING_SLOT, reflectionProbeBuffer);

		GLuint* gbuffer_textures = (GLuint*)cmdList.allocateSingleFrameMemory(3 * sizeof(GLuint));
		gbuffer_textures[0] = sceneContext.gbufferA;
		gbuffer_textures[1] = sceneContext.gbufferB;
		gbuffer_textures[2] = sceneContext.gbufferC;

		auto cubeArray = scene->reflectionProbeArrayTexture;

		cmdList.bindTextures(0, 3, gbuffer_textures);
		cmdList.bindTextureUnit(3, sceneContext.ssaoMap);
		cmdList.bindTextureUnit(4, sceneContext.getSkyPrefilterMapWithFallback());
		cmdList.bindTextureUnit(5, LightProbeBaker::get().getBRDFIntegrationMap_512());
		cmdList.bindTextureUnit(6, cubeArray != nullptr ? cubeArray->internal_getGLName() : 0);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);

		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_ReflectionProbe_BINDING_SLOT, 0);
		cmdList.bindTextures(0, 7, nullptr); // Fix a strange bug that IBL maps are randomly persistent across worlds.
	}

}
