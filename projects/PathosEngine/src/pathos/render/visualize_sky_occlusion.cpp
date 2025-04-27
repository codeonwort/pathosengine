#include "visualize_sky_occlusion.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/gl_debug_group.h"
#include "pathos/mesh/geometry.h"
#include "pathos/engine.h"
#include "pathos/console.h"

// cvars and shaders
namespace pathos {

	static ConsoleVariable<int32> cvar_visSkyOcclusion("r.visualizeSkyOcclusion", 0, "0 = disable, 1 = enable");

	struct UBO_VisualizeSkyOcclusion {
		static constexpr uint32 BINDING_SLOT = 1;

		uint32 numIrradianceVolumes;
		float  irradianceAtlasWidth;
		float  irradianceAtlasHeight;
		uint32 irradianceTileCountX;
		uint32 irradianceTileSize;
	};

	static constexpr uint32 SSBO_IrradianceVolume_BINDING_SLOT = 2; // Irradiance volumes
	static constexpr uint32 SSBO_IrradianceSH_BINDING_SLOT = 3; // Irradiance SH buffer

	class VisualizeSkyOcclusionFS : public ShaderStage {
	public:
		VisualizeSkyOcclusionFS() : ShaderStage(GL_FRAGMENT_SHADER, "VisualizeSkyOcclusionFS") {
			setFilepath("debugging/visualize_sky_occlusion.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_VisualizeSkyOcclusion, FullscreenVS, VisualizeSkyOcclusionFS);

}

namespace pathos {

	VisualizeSkyOcclusionPass::VisualizeSkyOcclusionPass() {}
	VisualizeSkyOcclusionPass::~VisualizeSkyOcclusionPass() {}

	void VisualizeSkyOcclusionPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		ubo.init<UBO_VisualizeSkyOcclusion>("UBO_VisualizeSkyOcclusion");
	}

	void VisualizeSkyOcclusionPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		ubo.safeDestroy();
	}

	void VisualizeSkyOcclusionPass::renderSkyOcclusion(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		const GLuint irradianceVolumeBuffer = scene->irradianceVolumeBuffer;
		MeshGeometry* fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		if (cvar_visSkyOcclusion.getInt() == 0 || irradianceVolumeBuffer == 0) {
			return;
		}

		SCOPED_DRAW_EVENT(VisualizeSkyOcclusion);

		// ----------------------------------------------------------
		// Prepare UBO & SSBO data

		UBO_VisualizeSkyOcclusion uboData;
		uboData.numIrradianceVolumes       = (uint32)scene->proxyList_irradianceVolume.size();
		uboData.irradianceAtlasWidth       = scene->irradianceAtlasWidth;
		uboData.irradianceAtlasHeight      = scene->irradianceAtlasHeight;
		uboData.irradianceTileCountX       = scene->irradianceTileCountX;
		uboData.irradianceTileSize         = scene->irradianceTileSize;

		// ----------------------------------------------------------
		// Rendering

		// Bind shader program
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VisualizeSkyOcclusion);
		cmdList.useProgram(program.getGLName());

		// Bind framebuffer
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		// Change render states
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.disable(GL_BLEND);
		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		// Bind buffers
		ubo.update(cmdList, UBO_VisualizeSkyOcclusion::BINDING_SLOT, &uboData);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_IrradianceVolume_BINDING_SLOT, irradianceVolumeBuffer);
		scene->irradianceSHBuffer->bindAsSSBO(cmdList, SSBO_IrradianceSH_BINDING_SLOT);

		// Bind textures
		cmdList.bindTextureUnit(0, sceneContext.gbufferA);
		cmdList.bindTextureUnit(1, sceneContext.gbufferB);
		cmdList.bindTextureUnit(2, sceneContext.gbufferC);
		cmdList.bindTextureUnit(3, sceneContext.sceneDepth);
		cmdList.bindTextureUnit(4, scene->irradianceAtlas);
		cmdList.bindTextureUnit(5, scene->depthProbeAtlas);

		fullscreenQuad->bindFullAttributesVAO(cmdList);
		fullscreenQuad->drawPrimitive(cmdList);
	}

}
