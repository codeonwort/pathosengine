#include "visualize_indirect_diffuse.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/gl_debug_group.h"
#include "pathos/mesh/geometry.h"
#include "pathos/engine.h"
#include "pathos/console.h"

// cvars and shaders
namespace pathos {

	static ConsoleVariable<int32> cvar_visIndirectDiffuse("r.visualizeIndirectDiffuse", 0, "0 = disable, 1 = sky occlusion");

	struct UBO_VisualizeIndirectDiffuse {
		static constexpr uint32 BINDING_SLOT = 1;

		uint32 numIrradianceVolumes;
		float  irradianceAtlasWidth;
		float  irradianceAtlasHeight;
		uint32 irradianceTileCountX;
		uint32 irradianceTileSize;
	};

	static constexpr uint32 SSBO_IrradianceVolume_BINDING_SLOT = 2; // Irradiance volumes
	static constexpr uint32 SSBO_IrradianceSH_BINDING_SLOT = 3; // Irradiance SH buffer

	class VisualizeIndirectDiffuseFS : public ShaderStage {
	public:
		VisualizeIndirectDiffuseFS() : ShaderStage(GL_FRAGMENT_SHADER, "VisualizeIndirectDiffuseFS") {
			setFilepath("debugging/visualize_indirect_diffuse.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_VisualizeIndirectDiffuse, FullscreenVS, VisualizeIndirectDiffuseFS);

}

namespace pathos {

	VisualizeIndirectDiffuse::VisualizeIndirectDiffuse() {}
	VisualizeIndirectDiffuse::~VisualizeIndirectDiffuse() {}

	void VisualizeIndirectDiffuse::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		ubo.init<UBO_VisualizeIndirectDiffuse>("UBO_VisualizeIndirectDiffuse");
	}

	void VisualizeIndirectDiffuse::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		ubo.safeDestroy();
	}

	void VisualizeIndirectDiffuse::renderVisualization(RenderCommandList& cmdList, SceneProxy* scene) {
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		const GLuint irradianceVolumeBuffer = scene->irradianceVolumeBuffer;
		MeshGeometry* fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		if (cvar_visIndirectDiffuse.getInt() == 0 || irradianceVolumeBuffer == 0) {
			return;
		}

		SCOPED_DRAW_EVENT(VisualizeIndirectDiffuse);

		// ----------------------------------------------------------
		// Prepare UBO & SSBO data

		UBO_VisualizeIndirectDiffuse uboData;
		uboData.numIrradianceVolumes       = (uint32)scene->proxyList_irradianceVolume.size();
		uboData.irradianceAtlasWidth       = scene->irradianceAtlasWidth;
		uboData.irradianceAtlasHeight      = scene->irradianceAtlasHeight;
		uboData.irradianceTileCountX       = scene->irradianceTileCountX;
		uboData.irradianceTileSize         = scene->irradianceTileSize;

		// ----------------------------------------------------------
		// Rendering

		// Bind shader program
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VisualizeIndirectDiffuse);
		cmdList.useProgram(program.getGLName());

		// Bind framebuffer
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		// Change render states
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.disable(GL_BLEND);
		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		// Bind buffers
		ubo.update(cmdList, UBO_VisualizeIndirectDiffuse::BINDING_SLOT, &uboData);
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
