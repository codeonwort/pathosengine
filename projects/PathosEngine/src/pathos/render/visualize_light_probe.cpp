#include "visualize_light_probe.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/light_probe_component.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	static ConsoleVariable<int32> cvar_visLightProbe(
		"r.visualizeLightProbe",
		0,
		"0 = disable, 1 = enable");
	static ConsoleVariable<float> cvar_visIrradianceProbeRadius(
		"r.visualizeLightProbe.irradianceRadius", 0.05f,
		"Radius of visualized irradiance probes (meters)");
	static ConsoleVariable<float> cvar_visRadianceProbeRadius(
		"r.visualizeLightProbe.radianceRadius", 1.0f,
		"Radius of visualized radiance probes (meters)");

	class VisualizeLightProbeVS : public ShaderStage {
	public:
		VisualizeLightProbeVS() : ShaderStage(GL_VERTEX_SHADER, "VisualizeLightProbeVS")
		{
			addDefine("VERTEX_SHADER", 1);
			setFilepath("visualize_light_probe.glsl");
		}
	};

	class VisualizeLightProbeFS : public ShaderStage {
	public:
		VisualizeLightProbeFS() : ShaderStage(GL_FRAGMENT_SHADER, "VisualizeLightProbeFS")
		{
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("visualize_light_probe.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_VisualizeLightProbe, VisualizeLightProbeVS, VisualizeLightProbeFS);

}

namespace pathos {

	struct UBO_VisualizeLightProbe {
		uint32 numIrradianceProbes;
		uint32 numRadianceProbes;
		float irradianceProbeRadius;
		float radianceProbeRadius;
	};
	struct IrradianceProbeStruct {
		vector3 positionWS;
		float captureRadius;
		vector4 uvBounds;
	};
	struct RadianceProbeStruct {
		vector3 positionWS;
		float captureRadius;
	};
	static constexpr uint32 UBO_BINDING_SLOT = 1;
	static constexpr uint32 SSBO_0_BINDING_SLOT = 2;
	static constexpr uint32 SSBO_1_BINDING_SLOT = 3;

	VisualizeLightProbePass::VisualizeLightProbePass() {}
	VisualizeLightProbePass::~VisualizeLightProbePass() {}

	void VisualizeLightProbePass::initializeResources(RenderCommandList& cmdList) {
		sphereGeom = new SphereGeometry(1.0f, 20);

		gRenderDevice->createFramebuffers(1, &fbo);

		ubo.init<UBO_VisualizeLightProbe>("UBO_VisualizeLightProbe");

		const uint32 maxIrradianceProbes = pathos::irradianceProbeTileCountX * pathos::irradianceProbeTileCountY;
		ssbo0.init(sizeof(IrradianceProbeStruct) * maxIrradianceProbes, "SSBO_0_VisualizeLightProbe");

		const uint32 maxRadianceProbes = pathos::radianceProbeMaxCount;
		ssbo1.init(sizeof(RadianceProbeStruct) * maxRadianceProbes, "SSBO_1_VisualizeLightProbe");
	}

	void VisualizeLightProbePass::releaseResources(RenderCommandList& cmdList) {
		delete sphereGeom;
		gRenderDevice->deleteFramebuffers(1, &fbo);
		ubo.safeDestroy();
		ssbo0.safeDestroy();
		ssbo1.safeDestroy();
	}

	void VisualizeLightProbePass::render(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera)
	{
		if (cvar_visLightProbe.getInt() == 0) {
			return;
		}

		//////////////////////////////////////////////////////////////////////////
		// Prepare UBO & SSBO data

		std::vector<IrradianceProbeStruct> ssbo0Data;
		for (const IrradianceProbeProxy* proxy : scene->proxyList_irradianceProbe) {
			if (proxy->hasValidTileID()) {
				IrradianceProbeStruct ssboItem{
					proxy->positionWS,
					proxy->captureRadius,
					proxy->irradianceTileBounds,
				};
				ssbo0Data.emplace_back(ssboItem);
			}
		}
		std::vector<RadianceProbeStruct> ssbo1Data;
		for (const RadianceProbeProxy* proxy : scene->proxyList_radianceProbe) {
			if (proxy->specularIBL != nullptr) {
				RadianceProbeStruct ssboItem{
					proxy->positionWS,
					proxy->captureRadius,
				};
				ssbo1Data.emplace_back(ssboItem);
			}
		}

		// Nothing to visualize
		if (ssbo0Data.size() == 0 && ssbo1Data.size() == 0) {
			return;
		}

		UBO_VisualizeLightProbe uboData;
		uboData.numIrradianceProbes = (uint32)ssbo0Data.size();
		uboData.numRadianceProbes = (uint32)ssbo1Data.size();
		uboData.irradianceProbeRadius = std::max(0.01f, cvar_visIrradianceProbeRadius.getFloat());
		uboData.radianceProbeRadius = std::max(0.01f, cvar_visRadianceProbeRadius.getFloat());

		//////////////////////////////////////////////////////////////////////////

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_VisualizeLightProbe);
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		cmdList.useProgram(program.getGLName());

		// Change render states
		cmdList.enable(GL_DEPTH_TEST);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);
		
		ubo.update(cmdList, UBO_BINDING_SLOT, &uboData);
		if (uboData.numIrradianceProbes > 0) {
			GLsizeiptr bytes = ssbo0Data.size() * sizeof(IrradianceProbeStruct);
			ssbo0.update(cmdList, SSBO_0_BINDING_SLOT, ssbo0Data.data(), bytes);
		}
		if (uboData.numRadianceProbes > 0) {
			GLsizeiptr bytes = ssbo1Data.size() * sizeof(RadianceProbeStruct);
			ssbo1.update(cmdList, SSBO_1_BINDING_SLOT, ssbo1Data.data(), bytes);
		}

		cmdList.textureParameteri(scene->irradianceAtlas, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cmdList.textureParameteri(scene->irradianceAtlas, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		cmdList.bindTextureUnit(0, scene->irradianceAtlas);
		cmdList.bindTextureUnit(1, sceneContext.localSpecularIBLs);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		
		uint32 instanceCount = uboData.numIrradianceProbes + uboData.numRadianceProbes;
		sphereGeom->activate_position_normal(cmdList);
		sphereGeom->activateIndexBuffer(cmdList);
		sphereGeom->drawPrimitive(cmdList, instanceCount);

		cmdList.disable(GL_DEPTH_TEST);
	}

}
