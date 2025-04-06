#include "visualize_light_probe.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/rhi/buffer.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/image_based_lighting.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/reflection_probe_component.h"
#include "pathos/scene/irradiance_volume_actor.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_visLightProbe("r.visualizeLightProbe", 0, "0 = disable, 1 = enable");
	static ConsoleVariable<float> cvar_visIrradianceProbeRadius("r.visualizeLightProbe.irradianceProbeRadius", 0.05f, "Radius of visualized probes in irradiance volumes (meters)");
	static ConsoleVariable<float> cvar_visReflectionProbeRadius("r.visualizeLightProbe.reflectionProbeRadius", 1.0f, "Radius of visualized reflection probes (meters)");

	struct UBO_VisualizeLightProbe {
		static constexpr uint32 BINDING_SLOT = 1;

		uint32 numIrradianceVolumes;
		uint32 totalIrradianceProbes;
		uint32 numReflectionProbes;
		float irradianceProbeRadius;

		float reflectionProbeRadius;
		float irradianceAtlasWidth;
		float irradianceAtlasHeight;
		uint32 irradianceTileCountX;

		uint32 irradianceTileSize;
	};

	static constexpr uint32 SSBO_IrradianceVolume_BINDING_SLOT = 2;
	static constexpr uint32 SSBO_ReflectionProbe_BINDING_SLOT = 3;

	class VisualizeLightProbeVS : public ShaderStage {
	public:
		VisualizeLightProbeVS() : ShaderStage(GL_VERTEX_SHADER, "VisualizeLightProbeVS")
		{
			addDefine("VERTEX_SHADER", 1);
			setFilepath("debugging/visualize_light_probe.glsl");
		}
	};

	class VisualizeLightProbeFS : public ShaderStage {
	public:
		VisualizeLightProbeFS() : ShaderStage(GL_FRAGMENT_SHADER, "VisualizeLightProbeFS")
		{
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("debugging/visualize_light_probe.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_VisualizeLightProbe, VisualizeLightProbeVS, VisualizeLightProbeFS);

}

namespace pathos {

	VisualizeLightProbePass::VisualizeLightProbePass() {}
	VisualizeLightProbePass::~VisualizeLightProbePass() {}

	void VisualizeLightProbePass::initializeResources(RenderCommandList& cmdList) {
		auto attribs = EPrimitiveInitOptions::CalculatePosition | EPrimitiveInitOptions::CalculateUV | EPrimitiveInitOptions::CalculateNormal;
		sphereGeom = new SphereGeometry(SphereGeometry::Input{ 1.0f, 20, attribs });
		gRenderDevice->createFramebuffers(1, &fbo);
		ubo.init<UBO_VisualizeLightProbe>("UBO_VisualizeLightProbe");
	}

	void VisualizeLightProbePass::releaseResources(RenderCommandList& cmdList) {
		delete sphereGeom;
		gRenderDevice->deleteFramebuffers(1, &fbo);
		ubo.safeDestroy();
	}

	void VisualizeLightProbePass::render(RenderCommandList& cmdList, SceneProxy* scene, Camera* camera) {
		if (cvar_visLightProbe.getInt() == 0) {
			return;
		}

		SCOPED_DRAW_EVENT(VisualizeLightProbe);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		const GLuint irradianceVolumeBuffer = scene->irradianceVolumeBuffer;
		const GLuint reflectionProbeBuffer = scene->reflectionProbeBuffer;

		// Nothing to visualize
		if (irradianceVolumeBuffer == 0 && reflectionProbeBuffer == 0) {
			return;
		}

		//////////////////////////////////////////////////////////////////////////
		// Prepare UBO & SSBO data

		uint32 totalIrradianceProbes = 0;
		for (size_t i = 0; i < scene->proxyList_irradianceVolume.size(); ++i)
		{
			totalIrradianceProbes += scene->proxyList_irradianceVolume[i]->numProbes;
		}

		UBO_VisualizeLightProbe uboData;
		uboData.numIrradianceVolumes = (uint32)scene->proxyList_irradianceVolume.size();
		uboData.totalIrradianceProbes = totalIrradianceProbes;
		uboData.numReflectionProbes = (uint32)scene->proxyList_reflectionProbe.size();
		uboData.irradianceProbeRadius = std::max(0.01f, cvar_visIrradianceProbeRadius.getFloat());
		uboData.reflectionProbeRadius = std::max(0.01f, cvar_visReflectionProbeRadius.getFloat());
		uboData.irradianceAtlasWidth = scene->irradianceAtlasWidth;
		uboData.irradianceAtlasHeight = scene->irradianceAtlasHeight;
		uboData.irradianceTileCountX = scene->irradianceTileCountX;
		uboData.irradianceTileSize = scene->irradianceTileSize;

		//////////////////////////////////////////////////////////////////////////

		// Bind shader program
		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VisualizeLightProbe);
		cmdList.useProgram(program.getGLName());

		// Bind framebuffer
		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		// Change render states
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		
		// Bind buffers
		ubo.update(cmdList, UBO_VisualizeLightProbe::BINDING_SLOT, &uboData);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_IrradianceVolume_BINDING_SLOT, irradianceVolumeBuffer);
		cmdList.bindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_ReflectionProbe_BINDING_SLOT, reflectionProbeBuffer);

		// Bind textures
		cmdList.textureParameteri(scene->irradianceAtlas, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(scene->irradianceAtlas, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		cmdList.bindTextureUnit(0, scene->irradianceAtlas);
		cmdList.bindTextureUnit(1, sceneContext.localSpecularIBLs);

		// Drawcall
		uint32 instanceCount = totalIrradianceProbes + uboData.numReflectionProbes;
		sphereGeom->bindFullAttributesVAO(cmdList);
		sphereGeom->drawPrimitive(cmdList, instanceCount);

		// Restore render states
		cmdList.disable(GL_DEPTH_TEST);
	}

}
