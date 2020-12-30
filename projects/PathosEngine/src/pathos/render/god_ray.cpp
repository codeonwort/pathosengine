#include "god_ray.h"
#include "pathos/render/render_deferred.h"
#include "pathos/render/render_device.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/console.h"
#include "pathos/util/log.h"
#include "pathos/util/math_lib.h"
#include "pathos/shader/shader.h"
#include "pathos/shader/shader_program.h"
#include "pathos/material/material.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/static_mesh_component.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	struct UBO_GodRayLightScattering {
		vector2 lightPos;
		float alphaDecay;
		float density;
	};

	class GodRayLightScatteringVS : public ShaderStage {
	public:
		GodRayLightScatteringVS() : ShaderStage(GL_VERTEX_SHADER, "GodRayLightScatteringVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	class GodRayLightScatteringFS : public ShaderStage {
	public:
		GodRayLightScatteringFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRayLightScatteringFS") {
			setFilepath("god_ray_fs.glsl");
		};
	};

	class GodRayBilateralSamplingVS : public ShaderStage {
	public:
		GodRayBilateralSamplingVS() : ShaderStage(GL_VERTEX_SHADER, "GodRayBilateralSamplingVS") {
			setFilepath("fullscreen_quad.glsl");
		}
	};

	template<bool horizontal>
	class GodRayBilateralSamplingFS : public ShaderStage {
	public:
		GodRayBilateralSamplingFS() : ShaderStage(GL_FRAGMENT_SHADER, "GodRayBilateralSamplingFS") {
			if (horizontal) {
				addDefine("HORIZONTAL 1");
			}
			addDefine("KERNEL_SIZE 5");
			//addDefine("TONAL_WEIGHT 1");
			setFilepath("two_pass_gaussian_blur.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_GodRayLightScattering, GodRayLightScatteringVS, GodRayLightScatteringFS);
	DEFINE_SHADER_PROGRAM2(Program_GodRayBilateralSamplingH, GodRayBilateralSamplingVS, GodRayBilateralSamplingFS<true>);
	DEFINE_SHADER_PROGRAM2(Program_GodRayBilateralSamplingV, GodRayBilateralSamplingVS, GodRayBilateralSamplingFS<false>);

}

namespace pathos {

	static ConsoleVariable<int32> cvar_godray_upsampling("r.godray.upsampling", 1, "Upsample god ray texture");
	static ConsoleVariable<float> cvar_godray_alphaDecay("r.godray.alphaDecay", 0.92f, "Alpha decay of god ray scattering (0.0 ~ 1.0)");
	static ConsoleVariable<float> cvar_godray_density("r.godray.density", 1.1f, "Density of god ray scattering (> 1.0)");

	GodRay::GodRay() {
	}

	GodRay::~GodRay() {
		CHECK(destroyed);
	}

	void GodRay::initializeResources(RenderCommandList& cmdList) {
		createFBO(cmdList);
		createShaders(cmdList);
		cmdList.genVertexArrays(1, &vao_dummy);
		uboLightScattering.init<UBO_GodRayLightScattering>();
	}

	void GodRay::releaseResources(RenderCommandList& cmdList)
	{
		if (!destroyed) {
			gRenderDevice->deleteVertexArrays(1, &vao_dummy);
			gRenderDevice->deleteFramebuffers(2, fbo);
			gRenderDevice->deleteProgram(program_silhouette);
		}
		destroyed = true;
	}

	void GodRay::createFBO(RenderCommandList& cmdList) {
		// generate fbo and textures
		gRenderDevice->createFramebuffers(2, fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo[GOD_RAY_SOURCE], -1, "FBO_GodRaySource");
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo[GOD_RAY_RESULT], -1, "FBO_GodRayResult");

		// #todo-framebuffer: Can't check completeness now
		//GLenum fboCompleteness0;
		//GLenum fboCompleteness1;

		// Silhoutte pass
		cmdList.namedFramebufferDrawBuffer(fbo[GOD_RAY_SOURCE], GL_COLOR_ATTACHMENT0);
		//cmdList.checkNamedFramebufferStatus(fbo[GOD_RAY_SOURCE], GL_DRAW_FRAMEBUFFER, &fboCompleteness0);

		// Light scattering pass
		cmdList.namedFramebufferDrawBuffer(fbo[GOD_RAY_RESULT], GL_COLOR_ATTACHMENT0);
		//cmdList.checkNamedFramebufferStatus(fbo[GOD_RAY_RESULT], GL_DRAW_FRAMEBUFFER, &fboCompleteness1);

		// #todo-cmd-list: Define a render command that checks framebuffer completeness rather than flushing here
		//cmdList.flushAllCommands();
		//if (fboCompleteness0 != GL_FRAMEBUFFER_COMPLETE) {
		//	LOG(LogFatal, "Failed to initialize fbo[GOD_RAY_SOURCE]");
		//	CHECK(0);
		//}
		//if (fboCompleteness1 != GL_FRAMEBUFFER_COMPLETE) {
		//	LOG(LogFatal, "Failed to initialize fbo[GOD_RAY_RESULT]");
		//	CHECK(0);
		//}

		gRenderDevice->createFramebuffers(1, &fboBlur1);
		cmdList.namedFramebufferDrawBuffer(fboBlur1, GL_COLOR_ATTACHMENT0);

		gRenderDevice->createFramebuffers(1, &fboBlur2);
		cmdList.namedFramebufferDrawBuffer(fboBlur2, GL_COLOR_ATTACHMENT0);
	}

	void GodRay::createShaders(RenderCommandList& cmdList) {
		std::string vshader, fshader;

		/////////////////////////////////////////////////////////////////////////////////
		// program - silhouette
		vshader = R"(
#version 430 core

layout (location = 0) in vec3 position;

layout (location = 0) uniform mat4 mvpTransform;

void main() {
	gl_Position = mvpTransform * vec4(position, 1.0);
}
)";

		fshader = R"(
#version 430 core

layout (location = 4) uniform vec3 color;

out vec4 out_color;

void main() {
	out_color = vec4(color, 1.0);
}
)";

		program_silhouette = pathos::createProgram(vshader, fshader, "GodRay_silhouette");
		uniform_mvp = 0;
		uniform_color = 4;
	}

	void GodRay::renderGodRay(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* fullscreenQuad, DeferredRenderer* renderer) {
		SCOPED_DRAW_EVENT(GodRay);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		// bind
		GLfloat transparent_black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GLfloat opaque_black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat opaque_white[] = { 1.0f, 0.5f, 0.0f, 1.0f };
		GLfloat depth_clear[] = { 0.0f };

		cmdList.namedFramebufferTexture(fbo[GOD_RAY_SOURCE], GL_COLOR_ATTACHMENT0, sceneContext.godRaySource, 0);
		cmdList.namedFramebufferTexture(fbo[GOD_RAY_SOURCE], GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);
		cmdList.namedFramebufferTexture(fbo[GOD_RAY_RESULT], GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);

		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_SOURCE], GL_COLOR, 0, transparent_black);
		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_RESULT], GL_COLOR, 0, transparent_black);

		// special case: no light source
		if (scene->godRaySource == nullptr || scene->godRaySource->getStaticMesh() == nullptr || scene->godRaySource->getStaticMesh()->getGeometries().size() == 0) {
			return;
		}

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthFunc(GL_EQUAL);

		// render silhouettes
		{
			SCOPED_DRAW_EVENT(RenderSilhouette);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[GOD_RAY_SOURCE]);
			cmdList.useProgram(program_silhouette);

			// Source
			std::vector<StaticMeshProxy*> sourceProxyList;
			scene->godRaySource->createRenderProxy_internal(sourceProxyList);
			for (StaticMeshProxy* sourceProxy : sourceProxyList) {
				renderSilhouette(cmdList, camera, sourceProxy, opaque_white);
			}
		}

		{
			SCOPED_DRAW_EVENT(DownsampleSilhouette);

			// Downsample
			cmdList.viewport(0, 0, sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);
			renderer->copyTexture(cmdList, sceneContext.godRaySource, sceneContext.godRayResultTemp);
		}

		// light scattering pass
		{
			SCOPED_DRAW_EVENT(LightScattering);

			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_GodRayLightScattering);

			vector3 lightPos = scene->godRaySource->getLocation();
			const glm::mat4 lightMVP = camera->getViewProjectionMatrix();
			auto lightPos_homo = lightMVP * glm::vec4(lightPos, 1.0f);
			lightPos = glm::vec3(lightPos_homo) / lightPos_homo.w;

			UBO_GodRayLightScattering uboData;
			uboData.lightPos.x = (lightPos.x + 1.0f) / 2.0f;
			uboData.lightPos.y = (lightPos.y + 1.0f) / 2.0f;
			uboData.alphaDecay = pathos::max(0.0f, pathos::min(1.0f, cvar_godray_alphaDecay.getFloat()));
			uboData.density    = pathos::max(1.0f, cvar_godray_density.getFloat());
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
			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.useProgram(program_vertical.getGLName());
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur2);
			cmdList.namedFramebufferTexture(fboBlur2, GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResultTemp);
			fullscreenQuad->drawPrimitive(cmdList);
		}
	}

	void GodRay::renderSilhouette(RenderCommandList& cmdList, Camera* camera, StaticMeshProxy* meshProxy, GLfloat* color) {
		// #todo-godray: Ignore translucent materials completely for now.
		// - Scaling the luminance proportional to the material's transmittance might produce a not so bad result.
		// - No idea how to deal with refraction :/
		if (meshProxy->material->getMaterialID() == MATERIAL_ID::TRANSLUCENT_SOLID_COLOR) {
			return;
		}

		cmdList.uniform3fv(uniform_color, 1, color);
		cmdList.uniformMatrix4fv(uniform_mvp, 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * meshProxy->modelMatrix));

		bool wireframe = meshProxy->material->getMaterialID() == MATERIAL_ID::WIREFRAME;
		if (wireframe) {
			cmdList.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		meshProxy->geometry->activate_position(cmdList);
		meshProxy->geometry->activateIndexBuffer(cmdList);
		meshProxy->geometry->drawPrimitive(cmdList);
		meshProxy->geometry->deactivate(cmdList);
		meshProxy->geometry->deactivateIndexBuffer(cmdList);

		if (wireframe) {
			cmdList.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

}
