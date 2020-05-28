#include "god_ray.h"
#include "scene_render_targets.h"
#include "pathos/console.h"
#include "pathos/util/log.h"
#include "pathos/shader/shader.h"
#include "pathos/material/material.h"
#include "pathos/mesh/geometry.h"
#include "pathos/mesh/mesh.h"
#include "pathos/mesh/static_mesh_component.h"

#include "badger/assertion/assertion.h"
#include "badger/types/matrix_types.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	static ConsoleVariable<int32> cvar_godray_upsampling("r.godray.upsampling", 1, "Upsample god ray texture");

	GodRay::GodRay() {
	}

	GodRay::~GodRay() {
		CHECK(destroyed);
	}

	void GodRay::initializeResources(RenderCommandList& cmdList) {
		createFBO(cmdList);
		createShaders(cmdList);
		cmdList.genVertexArrays(1, &vao_dummy);
	}

	void GodRay::releaseResources(RenderCommandList& cmdList)
	{
		if (!destroyed) {
			cmdList.deleteVertexArrays(1, &vao_dummy);
			cmdList.deleteFramebuffers(2, fbo);
			cmdList.deleteProgram(program_silhouette);
			cmdList.deleteProgram(program_godRay);
			cmdList.deleteProgram(program_blur1);
			cmdList.deleteProgram(program_blur2);
		}
		destroyed = true;
	}

	void GodRay::createFBO(RenderCommandList& cmdList) {
		// generate fbo and textures
		cmdList.createFramebuffers(2, fbo);
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

		cmdList.createFramebuffers(1, &fboBlur1);
		cmdList.namedFramebufferDrawBuffer(fboBlur1, GL_COLOR_ATTACHMENT0);

		cmdList.createFramebuffers(1, &fboBlur2);
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

		/////////////////////////////////////////////////////////////////////////////////
		// program - god ray
		{
			Shader vs(GL_VERTEX_SHADER, "GodRay_VS");
			Shader fs(GL_FRAGMENT_SHADER, "GodRay_FS");
			vs.loadSource("fullscreen_quad.glsl");
			fs.loadSource("god_ray_fs.glsl");

			program_godRay = pathos::createProgram(vs, fs, "GodRay");
			uniform_lightPos = 0;
		}

		// program - bilateral sampling
		{
			Shader vs_blur(GL_VERTEX_SHADER, "GodRay_VS_Blur_1");
			Shader fs_blur(GL_FRAGMENT_SHADER, "GodRay_FS_Blur_1");
			fs_blur.addDefine("HORIZONTAL 1");
			fs_blur.addDefine("KERNEL_SIZE 7");
			vs_blur.loadSource("fullscreen_quad.glsl");
			fs_blur.loadSource("two_pass_gaussian_blur.glsl");
			program_blur1 = pathos::createProgram(vs_blur, fs_blur, "GodRay_Blur_1");
		}
		{
			Shader vs_blur(GL_VERTEX_SHADER, "GodRay_VS_Blur_2");
			Shader fs_blur(GL_FRAGMENT_SHADER, "GodRay_FS_Blur_2");
			fs_blur.addDefine("HORIZONTAL 0");
			fs_blur.addDefine("KERNEL_SIZE 7");
			vs_blur.loadSource("fullscreen_quad.glsl");
			fs_blur.loadSource("two_pass_gaussian_blur.glsl");
			program_blur2 = pathos::createProgram(vs_blur, fs_blur, "GodRay_Blur_2");
		}
	}

	void GodRay::renderGodRay(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* fullscreenQuad) {
		SCOPED_DRAW_EVENT(GodRay);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		// bind
		GLfloat transparent_black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		GLfloat opaque_black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat opaque_white[] = { 1.0f, 0.5f, 0.0f, 1.0f };
		GLfloat depth_clear[] = { 1.0f };

		cmdList.namedFramebufferTexture(fbo[GOD_RAY_SOURCE], GL_COLOR_ATTACHMENT0, sceneContext.godRaySource, 0);
		cmdList.namedFramebufferTexture(fbo[GOD_RAY_RESULT], GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);

		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_SOURCE], GL_COLOR, 0, transparent_black);
		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_SOURCE], GL_DEPTH, 0, depth_clear);
		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_RESULT], GL_COLOR, 0, transparent_black);
		cmdList.clearNamedFramebufferfv(fbo[GOD_RAY_RESULT], GL_DEPTH, 0, depth_clear);

		cmdList.viewport(0, 0, sceneContext.sceneWidth / 2, sceneContext.sceneHeight / 2);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthFunc(GL_LEQUAL);

		// special case: no light source
		if (scene->godRaySource == nullptr || scene->godRaySource->getStaticMesh() == nullptr || scene->godRaySource->getStaticMesh()->getGeometries().size() == 0) {
			return;
		}

		// render silhouettes
		{
			SCOPED_DRAW_EVENT(RenderSilhouette);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[GOD_RAY_SOURCE]);
			cmdList.useProgram(program_silhouette);

			// Source
			std::vector<StaticMeshProxy*> sourceProxyList;
			scene->godRaySource->createRenderProxy_internal(scene, sourceProxyList); // #todo-godray: hack
			MeshGeometry* godRayGeometry = scene->godRaySource->getStaticMesh()->getGeometries()[0];
			for (StaticMeshProxy* sourceProxy : sourceProxyList) {
				renderSilhouette(cmdList, camera, sourceProxy, opaque_white);
			}

			// Occluders
			for (uint8 i = 0; i < static_cast<uint8>(MATERIAL_ID::NUM_MATERIAL_IDS); ++i) {
				const auto& proxyList = scene->proxyList_staticMesh[i];
				for (StaticMeshProxy* proxy : proxyList) {
					if (proxy->geometry == godRayGeometry) {
						continue;
					}
					renderSilhouette(cmdList, camera, proxy, opaque_black);
				}
			}
		}

		// light scattering pass
		{
			SCOPED_DRAW_EVENT(LightScattering);

			vector3 lightPos = scene->godRaySource->getLocation();
			const glm::mat4 lightMVP = camera->getViewProjectionMatrix();
			auto lightPos_homo = lightMVP * glm::vec4(lightPos, 1.0f);
			lightPos = glm::vec3(lightPos_homo) / lightPos_homo.w;
			GLfloat lightPos_2d[2] = { (lightPos.x + 1.0f) / 2.0f, (lightPos.y + 1.0f) / 2.0f };

			cmdList.bindVertexArray(vao_dummy);

			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[GOD_RAY_RESULT]);
			cmdList.useProgram(program_godRay);
			cmdList.bindTextureUnit(0, sceneContext.godRaySource);
			cmdList.uniform2fv(uniform_lightPos, 1, lightPos_2d);
			cmdList.drawArrays(GL_TRIANGLE_STRIP, 0, 4); // gl error?

			// unbind
			cmdList.bindVertexArray(0);
			cmdList.useProgram(0);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		// Bilateral sampling
		if (cvar_godray_upsampling.getInt() != 0)
		{
			SCOPED_DRAW_EVENT(BilateralSampling);

			cmdList.useProgram(program_blur1);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur1);
			cmdList.namedFramebufferTexture(fboBlur1, GL_COLOR_ATTACHMENT0, sceneContext.godRayResultTemp, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResult);
			fullscreenQuad->activate_position_uv(cmdList);
			fullscreenQuad->activateIndexBuffer(cmdList);
			fullscreenQuad->drawPrimitive(cmdList);

			cmdList.useProgram(program_blur2);
			cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBlur2);
			cmdList.namedFramebufferTexture(fboBlur2, GL_COLOR_ATTACHMENT0, sceneContext.godRayResult, 0);
			cmdList.bindTextureUnit(0, sceneContext.godRayResultTemp);
			fullscreenQuad->drawPrimitive(cmdList);
		}
	}

	void GodRay::renderSilhouette(RenderCommandList& cmdList, Camera* camera, StaticMeshProxy* meshProxy, GLfloat* color) {
		// #todo-godray: Ignore translucent materials for now
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
