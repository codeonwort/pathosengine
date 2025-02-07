#include "god_ray.h"
#include "pathos/engine_policy.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/scene_renderer.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/fullscreen_util.h"
#include "pathos/material/material.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/static_mesh.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/console.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"
#include "badger/math/minmax.h"

static ConsoleVariable<int32> cvar_godray_upsampling("r.godray.upsampling", 1, "Upsample god ray texture");
static ConsoleVariable<float> cvar_godray_alphaDecay("r.godray.alphaDecay", 0.96f, "Alpha decay of god ray scattering (0.0 ~ 1.0)");
static ConsoleVariable<float> cvar_godray_density("r.godray.density", 1.2f, "Density of god ray scattering (> 1.0)");

namespace pathos {

	struct UBO_GodRaySilhouette {
		static constexpr uint32 BINDING_INDEX = 1;

		matrix4 modelTransform;
		vector3 godRayColor;
	};

	class GodRaySilhouetteVS : public ShaderStage {
	public:
		GodRaySilhouetteVS() : ShaderStage(GL_VERTEX_SHADER, "GodRaySilhouetteVS") {
			addDefine("VERTEX_SHADER 1");
			setFilepath("godray/god_ray_silhouette.glsl");
		}
	};

	class GodRaySilhouetteFS : public ShaderStage {
	public:
		GodRaySilhouetteFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRaySilhouetteFS") {
			addDefine("FRAGMENT_SHADER 1");
			setFilepath("godray/god_ray_silhouette.glsl");
		}
	};

	struct UBO_GodRayLightScattering {
		static constexpr uint32 BINDING_INDEX = 1;

		vector2 lightPos;
		float alphaDecay;
		float density;
		float lightIntensity;
	};

	class GodRayLightScatteringFS : public ShaderStage {
	public:
		GodRayLightScatteringFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRayLightScatteringFS") {
			setFilepath("godray/god_ray_fs.glsl");
		};
	};

	template<bool horizontal>
	class GodRayBilateralSamplingFS : public ShaderStage {
	public:
		GodRayBilateralSamplingFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRayBilateralSamplingFS") {
			if (horizontal) {
				addDefine("HORIZONTAL", 1);
			}
			addDefine("KERNEL_SIZE", 5);
			//addDefine("TONAL_WEIGHT", 1);
			setFilepath("godray/two_pass_gaussian_blur.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_GodRaySilhouette, GodRaySilhouetteVS, GodRaySilhouetteFS);
	DEFINE_SHADER_PROGRAM2(Program_GodRayLightScattering, FullscreenVS, GodRayLightScatteringFS);
	DEFINE_SHADER_PROGRAM2(Program_GodRayBilateralSamplingH, FullscreenVS, GodRayBilateralSamplingFS<true>);
	DEFINE_SHADER_PROGRAM2(Program_GodRayBilateralSamplingV, FullscreenVS, GodRayBilateralSamplingFS<false>);

	class GodRayPostFS : public ShaderStage {
	public:
		GodRayPostFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRayPostFS") {
			setFilepath("godray/god_ray_post.glsl");
		}
	};
	DEFINE_SHADER_PROGRAM2(Program_GodRayPost, FullscreenVS, GodRayPostFS);
}

namespace pathos {

	GodRay::GodRay() {}

	GodRay::~GodRay() {
		CHECK(destroyed);
	}

	void GodRay::initializeResources(RenderCommandList& cmdList) {
		createFBO(cmdList);
		uboSilhouette.init<UBO_GodRaySilhouette>("UBO_GodRaySilhouette");
		uboLightScattering.init<UBO_GodRayLightScattering>("UBO_GodRayLightScattering");
	}

	void GodRay::releaseResources(RenderCommandList& cmdList) {
		if (!destroyed) {
			GLuint FBOs[] = { fboSilhouette, fboLight, fboBlurH, fboBlurV, fboPost };
			gRenderDevice->deleteFramebuffers(_countof(FBOs), FBOs);
			destroyed = true;
		}
	}

	void GodRay::createFBO(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fboSilhouette);
		gRenderDevice->createFramebuffers(1, &fboLight);
		gRenderDevice->createFramebuffers(1, &fboBlurH);
		gRenderDevice->createFramebuffers(1, &fboBlurV);
		gRenderDevice->createFramebuffers(1, &fboPost);

		// Silhoutte
		cmdList.namedFramebufferDrawBuffer(fboSilhouette, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fboSilhouette, -1, "FBO_GodRaySilhouette");

		// Light scattering
		cmdList.namedFramebufferDrawBuffer(fboLight, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fboLight, -1, "FBO_GodRayLight");

		// Blur
		cmdList.namedFramebufferDrawBuffer(fboBlurH, GL_COLOR_ATTACHMENT0);
		cmdList.namedFramebufferDrawBuffer(fboBlurV, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fboBlurH, -1, "FBO_GodRayBlurH");
		cmdList.objectLabel(GL_FRAMEBUFFER, fboBlurV, -1, "FBO_GodRayBlurV");

		// Post
		cmdList.namedFramebufferDrawBuffer(fboPost, GL_COLOR_ATTACHMENT0);
		cmdList.objectLabel(GL_FRAMEBUFFER, fboPost, -1, "FBO_GodRayPost");
	}

	void GodRay::renderGodRay(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		SceneRenderer* renderer)
	{
		SCOPED_DRAW_EVENT(GodRay);

		// Special case: no light source
		if (!isPassEnabled(scene)) {
			return;
		}

		godRayColor = scene->godRayColor;
		godRayIntensity = scene->godRayIntensity;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		MeshGeometry* fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

		// Bind framebuffer
		GLfloat transparent_black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GLfloat opaque_black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat depth_clear[] = { 0.0f };

		cmdList.namedFramebufferTexture(fboSilhouette, GL_COLOR_ATTACHMENT0, sceneContext.godRaySource, 0);
		cmdList.namedFramebufferTexture(fboSilhouette, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);
		cmdList.namedFramebufferTexture(fboLight, GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);

		pathos::checkFramebufferStatus(cmdList, fboSilhouette, "fboSilhouette is invalid");
		pathos::checkFramebufferStatus(cmdList, fboLight, "fboLight is invalid");

		cmdList.clearNamedFramebufferfv(fboSilhouette, GL_COLOR, 0, transparent_black);
		cmdList.clearNamedFramebufferfv(fboLight, GL_COLOR, 0, transparent_black);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthFunc(GL_EQUAL);
		if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
			cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		}

		// Render silhouettes
		{
			SCOPED_DRAW_EVENT(RenderSilhouette);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_GodRaySilhouette);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboSilhouette);
			cmdList.useProgram(program.getGLName());

			// Source
			// #todo-godray: Flickering due to TAA jitter
			std::vector<StaticMeshProxy*>& sourceProxyList = scene->godRayMeshes;
			for (StaticMeshProxy* sourceProxy : sourceProxyList) {
				renderSilhouette(cmdList, camera, sourceProxy);
			}

			cmdList.bindVertexArray(0); // Cleanup potentially remaining VAO binding
		}

		{
			SCOPED_DRAW_EVENT(DownsampleSilhouette);

			// Downsample
			renderer->copyTexture(cmdList, sceneContext.godRaySource, sceneContext.godRayResultTemp,
				sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);
		}

		// Light scattering pass
		{
			SCOPED_DRAW_EVENT(LightScattering);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_GodRayLightScattering);

			vector3 lightPos = scene->godRayLocation;
			const matrix4 lightMVP = camera->getViewProjectionMatrix();
			vector4 lightPosH = lightMVP * vector4(lightPos, 1.0f);
			lightPos = vector3(lightPosH) / lightPosH.w; // clip space

			UBO_GodRayLightScattering uboData;
			uboData.lightPos.x     = (lightPos.x + 1.0f) / 2.0f;
			uboData.lightPos.y     = (lightPos.y + 1.0f) / 2.0f;
			uboData.alphaDecay     = badger::clamp(0.0f, cvar_godray_alphaDecay.getFloat(), 1.0f);
			uboData.density        = badger::max(1.0f, cvar_godray_density.getFloat());
			uboData.lightIntensity = std::pow(godRayIntensity, 0.333f);
			uboLightScattering.update(cmdList, UBO_GodRayLightScattering::BINDING_INDEX, &uboData);

			cmdList.useProgram(program.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboLight);

			cmdList.viewport(0, 0, sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);

			cmdList.namedFramebufferTexture(fboLight, GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResultTemp);
			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.namedFramebufferTexture(fboLight, GL_COLOR_ATTACHMENT0, sceneContext.godRayResultTemp, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResult);
			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.namedFramebufferTexture(fboLight, GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResultTemp);
			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);
		}

		// #todo-godray: This is just gaussian blur. Range filter kernel is needed.
		// Bilateral sampling
		if (cvar_godray_upsampling.getInt() != 0)
		{
			SCOPED_DRAW_EVENT(BilateralSampling);

			ShaderProgram& program_horizontal = FIND_SHADER_PROGRAM(Program_GodRayBilateralSamplingH);
			ShaderProgram& program_vertical = FIND_SHADER_PROGRAM(Program_GodRayBilateralSamplingV);

			cmdList.useProgram(program_horizontal.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlurH);
			cmdList.namedFramebufferTexture(fboBlurH, GL_COLOR_ATTACHMENT0, sceneContext.godRayResultTemp, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResult);
			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.useProgram(program_vertical.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlurV);
			cmdList.namedFramebufferTexture(fboBlurV, GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResultTemp);
			fullscreenQuad->drawPrimitive(cmdList);
		}
	}

	void GodRay::renderGodRayPost(RenderCommandList& cmdList, SceneProxy* scene) {
		if (isPassEnabled(scene)) {
			SCOPED_DRAW_EVENT(GodRayPost);

			SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_GodRayPost);
			MeshGeometry* fullscreenQuad = gEngine->getSystemGeometryUnitPlane();

			cmdList.useProgram(program.getGLName());

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboPost);
			cmdList.namedFramebufferTexture(fboPost, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);

			pathos::checkFramebufferStatus(cmdList, fboPost, "[GodRayPost] FBO is invalid");

			// Set render states
			cmdList.disable(GL_DEPTH_TEST);
			cmdList.enable(GL_BLEND);
			cmdList.blendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);

			cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

			cmdList.bindTextureUnit(0, sceneContext.godRayResult);

			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			// Restore bindings
			cmdList.bindTextureUnit(0, 0);

			// Restore render states
			cmdList.disable(GL_BLEND);
			cmdList.namedFramebufferTexture(fboPost, GL_COLOR_ATTACHMENT0, 0, 0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
	}

	void GodRay::renderSilhouette(RenderCommandList& cmdList, Camera* camera, StaticMeshProxy* meshProxy) {
		// #todo-godray: Ignore translucent materials completely for now.
		// - Scaling the luminance proportional to the material's transmittance might produce a not so bad result.
		// - No idea how to deal with refraction :/
		if (meshProxy->material->getShadingModel() == EMaterialShadingModel::TRANSLUCENT) {
			return;
		}

		UBO_GodRaySilhouette uboData;
		uboData.modelTransform = meshProxy->modelMatrix;
		uboData.godRayColor = godRayColor;
		uboSilhouette.update(cmdList, UBO_GodRaySilhouette::BINDING_INDEX, &uboData);

		const bool wireframe = meshProxy->material->bWireframe;
		if (wireframe) {
			cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		meshProxy->geometry->bindPositionOnlyVAO(cmdList);
		meshProxy->geometry->drawPrimitive(cmdList);

		if (wireframe) {
			cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	bool GodRay::isPassEnabled(SceneProxy* scene) const {
		return scene->isGodRayValid();
	}

}
