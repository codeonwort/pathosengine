#include "skybox.h"
#include "pathos/engine_policy.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/image_based_lighting_baker.h"
#include "pathos/material/material.h"
#include "pathos/material/material_shader.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/skybox_component.h"

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include <string>

namespace pathos {
	
	class SkyboxVS : public ShaderStage {
	public:
		SkyboxVS() : ShaderStage(GL_VERTEX_SHADER, "SkyboxVS") {
			addDefine("VERTEX_SHADER", 1);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z", 1);
			}
			setFilepath("skybox.glsl");
		}
	};

	class SkyboxFS : public ShaderStage {
	public:
		SkyboxFS() : ShaderStage(GL_FRAGMENT_SHADER, "SkyboxFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("skybox.glsl");
		}
	};

	struct UBO_Skybox {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4 viewProj;
		float skyboxLOD;
	};

	DEFINE_SHADER_PROGRAM2(Program_Skybox, SkyboxVS, SkyboxFS);

}

namespace pathos {

	void SkyboxPass::initializeResources(RenderCommandList& cmdList) {
		cubeGeometry = gEngine->getSystemGeometryUnitCube();

		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_Skybox");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		gRenderDevice->createFramebuffers(1, &fboCube);
		cmdList.objectLabel(GL_FRAMEBUFFER, fboCube, -1, "FBO_SkyMaterialToCube");
		cmdList.namedFramebufferDrawBuffer(fboCube, GL_COLOR_ATTACHMENT0);

		scratchCubemapSize = pathos::SKY_PREFILTER_MAP_MIN_SIZE;
		gRenderDevice->createTextures(GL_TEXTURE_CUBE_MAP, 1, &scratchCubemapTexture);
		cmdList.textureStorage2D(scratchCubemapTexture, 1, GL_RGBA16F, scratchCubemapSize, scratchCubemapSize);

		matrix4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		matrix4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};
		for (int32 i = 0; i < 6; ++i) {
			cubeTransforms[i] = captureProjection * captureViews[i];
		}

		ubo.init<UBO_Skybox>("UBO_Skybox");
		uboPerObject.init<Material::UBO_PerObject>("UBO_SkyMaterialToCube");
	}

	void SkyboxPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
		gRenderDevice->deleteFramebuffers(1, &fboCube);
		gRenderDevice->deleteTextures(1, &scratchCubemapTexture);
	}

	void SkyboxPass::renderSkybox(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(Skybox);

		renderSkyboxToScreen(cmdList, scene);
		if (scene->sceneProxySource == SceneProxySource::MainScene && scene->skybox->bLightingDirty) {
			Texture* skyboxTexture = scene->skybox->texture;
			GLuint inputCubemap = skyboxTexture ? skyboxTexture->internal_getGLName() : 0;
			if (scene->skybox->bUseCubemapTexture == false) {
				renderSkyMaterialToCubemap(cmdList, scene);
				inputCubemap = scratchCubemapTexture;
			}
			renderSkyIrradianceMap(cmdList, scene, inputCubemap);
			renderSkyPreftilerMap(cmdList, scene, inputCubemap);
		}
	}

	void SkyboxPass::renderSkyboxToScreen(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyboxToScreen);

		const Camera& camera = scene->camera;
		SkyboxProxy* skybox = scene->skybox;

		Material* skyMaterial = nullptr;
		MaterialShader* skyMaterialShader = nullptr;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint programName = 0;
		if (skybox->bUseCubemapTexture) {
			ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Skybox);
			programName = program.getGLName();
		} else {
			skyMaterial = skybox->skyboxMaterial;
			skyMaterialShader = skyMaterial->internal_getMaterialShader();
			programName = skyMaterialShader->program->getGLName();
		}
		CHECK(programName != 0 && programName != 0xffffffff);
		cmdList.useProgram(programName);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		// Should use UBO_PerObject for sky material, but the layout matches accidentally.
		UBO_Skybox uboData;
		{
			matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
			matrix4 proj = camera.getProjectionMatrix();
			uboData.viewProj = proj * view;
			uboData.skyboxLOD = skybox->textureLod;
		}
		ubo.update(cmdList, UBO_Skybox::BINDING_POINT, &uboData);

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		cmdList.cullFace(GL_FRONT);

		if (skybox->bUseCubemapTexture) {
			cmdList.bindTextureUnit(0, skybox->texture->internal_getGLName());
		} else {
			if (skyMaterialShader->uboTotalBytes > 0) {
				uint8* uboMemory = reinterpret_cast<uint8*>(cmdList.allocateSingleFrameMemory(skyMaterialShader->uboTotalBytes));
				skyMaterial->internal_fillUniformBuffer(uboMemory);
				skyMaterialShader->uboMaterial.update(cmdList, skyMaterialShader->uboBindingPoint, uboMemory);
			}
			for (const MaterialTextureParameter& mtp : skyMaterial->internal_getTextureParameters()) {
				cmdList.bindTextureUnit(mtp.binding, mtp.glTexture);
			}
		}

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		skybox->cube->activate_position(cmdList);
		skybox->cube->activateIndexBuffer(cmdList);
		skybox->cube->drawPrimitive(cmdList);

		cmdList.cullFace(GL_BACK);
	}

	void SkyboxPass::renderSkyMaterialToCubemap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyMaterialToCubemap);

		GLuint programName = scene->skybox->skyboxMaterial->internal_getMaterialShader()->program->getGLName();
		cmdList.useProgram(programName);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboCube);

		cmdList.viewport(0, 0, scratchCubemapSize, scratchCubemapSize);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cubeGeometry->activate_position(cmdList);
		cubeGeometry->activateIndexBuffer(cmdList);

		for (int32 i = 0; i < 6; ++i) {
			cmdList.namedFramebufferTextureLayer(fboCube, GL_COLOR_ATTACHMENT0, scratchCubemapTexture, 0, i);

			// Hack UBO_PerObject
			Material::UBO_PerObject uboData{ cubeTransforms[i], cubeTransforms[i] };
			uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);

			cubeGeometry->drawPrimitive(cmdList);
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void SkyboxPass::renderSkyIrradianceMap(RenderCommandList& cmdList, SceneProxy* scene, GLuint inputCubemap) {
		SCOPED_DRAW_EVENT(SkyboxToIrradianceMap);

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		GLuint targetCubemap = sceneContext.skyIrradianceMap;
		uint32 targetSize = pathos::SKY_IRRADIANCE_MAP_SIZE;

		ImageBasedLightingBaker::bakeSkyIrradianceMap_renderThread(
			cmdList,
			inputCubemap,
			targetCubemap,
			targetSize);
	}

	void SkyboxPass::renderSkyPreftilerMap(RenderCommandList& cmdList, SceneProxy* scene, GLuint inputCubemap) {
		SCOPED_DRAW_EVENT(SkyboxToPrefilterMap);

		constexpr uint32 targetCubemapSize = pathos::SKY_PREFILTER_MAP_DEFAULT_SIZE;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		sceneContext.reallocSkyPrefilterMap(cmdList, targetCubemapSize);

		ImageBasedLightingBaker::bakeSpecularIBL_renderThread(
			cmdList,
			inputCubemap,
			targetCubemapSize,
			sceneContext.skyPrefilterMapMipCount,
			sceneContext.skyPrefilteredMap);
	}

}
