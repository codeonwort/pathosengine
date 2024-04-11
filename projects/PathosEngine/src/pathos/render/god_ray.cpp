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
#include "pathos/mesh/mesh.h"
#include "pathos/scene/static_mesh_component.h"
#include "pathos/console.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"
#include "badger/math/minmax.h"

static ConsoleVariable<int32> cvar_godray_upsampling("r.godray.upsampling", 1, "Upsample god ray texture");
static ConsoleVariable<float> cvar_godray_alphaDecay("r.godray.alphaDecay", 0.92f, "Alpha decay of god ray scattering (0.0 ~ 1.0)");
static ConsoleVariable<float> cvar_godray_density("r.godray.density", 1.1f, "Density of god ray scattering (> 1.0)");

namespace pathos {

	struct UBO_GodRaySilhouette {
		matrix4 modelTransform;
		vector3 godRayColor;
	};

	class GodRaySilhouetteVS : public ShaderStage {
	public:
		GodRaySilhouetteVS() : ShaderStage(GL_VERTEX_SHADER, "GodRaySilhouetteVS") {
			addDefine("VERTEX_SHADER 1");
			setFilepath("god_ray_silhouette.glsl");
		}
	};

	class GodRaySilhouetteFS : public ShaderStage {
	public:
		GodRaySilhouetteFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRaySilhouetteFS") {
			addDefine("FRAGMENT_SHADER 1");
			setFilepath("god_ray_silhouette.glsl");
		}
	};

	struct UBO_GodRayLightScattering {
		vector2 lightPos;
		float alphaDecay;
		float density;
	};

	class GodRayLightScatteringFS : public ShaderStage {
	public:
		GodRayLightScatteringFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRayLightScatteringFS") {
			setFilepath("god_ray_fs.glsl");
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
			setFilepath("two_pass_gaussian_blur.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_GodRaySilhouette, GodRaySilhouetteVS, GodRaySilhouetteFS);
	DEFINE_SHADER_PROGRAM2(Program_GodRayLightScattering, FullscreenVS, GodRayLightScatteringFS);
	DEFINE_SHADER_PROGRAM2(Program_GodRayBilateralSamplingH, FullscreenVS, GodRayBilateralSamplingFS<true>);
	DEFINE_SHADER_PROGRAM2(Program_GodRayBilateralSamplingV, FullscreenVS, GodRayBilateralSamplingFS<false>);

}

namespace pathos {

	GodRay::GodRay()
		: godRayColor(vector3(1.0f, 0.5f, 0.0f))
	{
	}

	GodRay::~GodRay() {
		CHECK(destroyed);
	}

	void GodRay::initializeResources(RenderCommandList& cmdList) {
		createFBO(cmdList);
		cmdList.genVertexArrays(1, &vao_dummy);
		uboSilhouette.init<UBO_GodRaySilhouette>();
		uboLightScattering.init<UBO_GodRayLightScattering>();
	}

	void GodRay::releaseResources(RenderCommandList& cmdList)
	{
		if (!destroyed) {
			gRenderDevice->deleteVertexArrays(1, &vao_dummy);
			gRenderDevice->deleteFramebuffers(2, fbo);
		}
		destroyed = true;
	}

	void GodRay::createFBO(RenderCommandList& cmdList) {
		// generate fbo and textures
		gRenderDevice->createFramebuffers(2, fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo[GOD_RAY_SOURCE], -1, "FBO_GodRaySource");
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo[GOD_RAY_RESULT], -1, "FBO_GodRayResult");

		// Silhoutte pass
		cmdList.namedFramebufferDrawBuffer(fbo[GOD_RAY_SOURCE], GL_COLOR_ATTACHMENT0);

		// Light scattering pass
		cmdList.namedFramebufferDrawBuffer(fbo[GOD_RAY_RESULT], GL_COLOR_ATTACHMENT0);
		
		gRenderDevice->createFramebuffers(1, &fboBlur1);
		cmdList.namedFramebufferDrawBuffer(fboBlur1, GL_COLOR_ATTACHMENT0);

		gRenderDevice->createFramebuffers(1, &fboBlur2);
		cmdList.namedFramebufferDrawBuffer(fboBlur2, GL_COLOR_ATTACHMENT0);
	}

	void GodRay::renderGodRay(
		RenderCommandList& cmdList,
		SceneProxy* scene,
		Camera* camera,
		MeshGeometry* fullscreenQuad,
		SceneRenderer* renderer)
	{
		SCOPED_DRAW_EVENT(GodRay);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		// bind
		GLfloat transparent_black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GLfloat opaque_black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat depth_clear[] = { 0.0f };

		cmdList.namedFramebufferTexture(fbo[GOD_RAY_SOURCE], GL_COLOR_ATTACHMENT0, sceneContext.godRaySource, 0);
		cmdList.namedFramebufferTexture(fbo[GOD_RAY_SOURCE], GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);
		cmdList.namedFramebufferTexture(fbo[GOD_RAY_RESULT], GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);

		pathos::checkFramebufferStatus(cmdList, fbo[GOD_RAY_SOURCE], "fbo[GOD_RAY_SOURCE] is invalid");
		pathos::checkFramebufferStatus(cmdList, fbo[GOD_RAY_RESULT], "fbo[GOD_RAY_RESULT] is invalid");

		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_SOURCE], GL_COLOR, 0, transparent_black);
		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_RESULT], GL_COLOR, 0, transparent_black);

		// special case: no light source
		if (!scene->isGodRayValid()) {
			return;
		}

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthFunc(GL_EQUAL);
		if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
			cmdList.clipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		}

		// render silhouettes
		{
			SCOPED_DRAW_EVENT(RenderSilhouette);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_GodRaySilhouette);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[GOD_RAY_SOURCE]);
			cmdList.useProgram(program.getGLName());

			// Source
			std::vector<StaticMeshProxy*>& sourceProxyList = scene->godRayMeshes;
			for (StaticMeshProxy* sourceProxy : sourceProxyList) {
				renderSilhouette(cmdList, camera, sourceProxy);
			}
		}

		{
			SCOPED_DRAW_EVENT(DownsampleSilhouette);

			// Downsample
			renderer->copyTexture(cmdList, sceneContext.godRaySource, sceneContext.godRayResultTemp,
				sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);
		}

		// light scattering pass
		{
			SCOPED_DRAW_EVENT(LightScattering);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_GodRayLightScattering);

			vector3 lightPos = scene->godRayLocation;
			const glm::mat4 lightMVP = camera->getViewProjectionMatrix();
			auto lightPos_homo = lightMVP * glm::vec4(lightPos, 1.0f);
			lightPos = vector3(lightPos_homo) / lightPos_homo.w;

			UBO_GodRayLightScattering uboData;
			uboData.lightPos.x = (lightPos.x + 1.0f) / 2.0f;
			uboData.lightPos.y = (lightPos.y + 1.0f) / 2.0f;
			uboData.alphaDecay = badger::clamp(0.0f, cvar_godray_alphaDecay.getFloat(), 1.0f);
			uboData.density    = badger::max(1.0f, cvar_godray_density.getFloat());
			uboLightScattering.update(cmdList, 1, &uboData);

			cmdList.bindVertexArray(vao_dummy);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[GOD_RAY_RESULT]);
			cmdList.useProgram(program.getGLName());
			cmdList.bindTextureUnit(0, sceneContext.godRayResultTemp);
			cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4);

			cmdList.bindVertexArray(0);
			cmdList.useProgram(0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		// #todo-godray: This is just gaussian blur. Range filter kernel is needed.
		// Bilateral sampling
		if (cvar_godray_upsampling.getInt() != 0)
		{
			SCOPED_DRAW_EVENT(BilateralSampling);

			ShaderProgram& program_horizontal = FIND_SHADER_PROGRAM(Program_GodRayBilateralSamplingH);
			ShaderProgram& program_vertical = FIND_SHADER_PROGRAM(Program_GodRayBilateralSamplingV);

			cmdList.useProgram(program_horizontal.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur1);
			cmdList.namedFramebufferTexture(fboBlur1, GL_COLOR_ATTACHMENT0, sceneContext.godRayResultTemp, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResult);
			fullscreenQuad->bindFullAttributesVAO(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.useProgram(program_vertical.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur2);
			cmdList.namedFramebufferTexture(fboBlur2, GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResultTemp);
			fullscreenQuad->drawPrimitive(cmdList);
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
		uboSilhouette.update(cmdList, 1, &uboData);

		const bool wireframe = meshProxy->material->bWireframe;
		if (wireframe) {
			cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		meshProxy->geometry->bindPositionOnlyVAO(cmdList);
		meshProxy->geometry->drawPrimitive(cmdList);
		meshProxy->geometry->unbindVAO(cmdList);

		if (wireframe) {
			cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

}
