#include "skybox.h"
#include "pathos/engine_policy.h"
#include "pathos/rhi/texture.h"
#include "pathos/rhi/shader_program.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/light_probe_baker.h"
#include "pathos/material/material.h"
#include "pathos/material/material_shader.h"
#include "pathos/mesh/geometry_primitive.h"
#include "pathos/scene/camera.h"
#include "pathos/scene/skybox_component.h"

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include "badger/math/bits.h"
#include <string>

namespace pathos {
	
	class SkyboxVS : public ShaderStage {
	public:
		SkyboxVS() : ShaderStage(GL_VERTEX_SHADER, "SkyboxVS") {
			addDefine("VERTEX_SHADER", 1);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z", 1);
			}
			setFilepath("sky/skybox.glsl");
		}
	};

	class SkyboxFS : public ShaderStage {
	public:
		SkyboxFS() : ShaderStage(GL_FRAGMENT_SHADER, "SkyboxFS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("sky/skybox.glsl");
		}
	};

	struct UBO_Skybox {
		static constexpr uint32 BINDING_POINT = 1;

		matrix4 viewProj;
		float skyboxLOD;
		float intensityMultiplier;
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

		const uint32 reflectionCubeSize = pathos::SKY_PREFILTER_MAP_MIN_SIZE;
		const uint32 ambientCubeSize = pathos::SKY_AMBIENT_CUBEMAP_SIZE;

		const uint32 mipLevels = 1 + badger::ctz(reflectionCubeSize) - badger::ctz(ambientCubeSize);
		reflectionCubemap = new Texture(TextureCreateParams::cubemap(reflectionCubeSize, GL_RGBA16F, mipLevels));
		reflectionCubemap->createGPUResource_renderThread(cmdList);

		ambientCubemap = new Texture(TextureCreateParams::cubemap(ambientCubeSize, GL_RGBA16F, 1));
		ambientCubemap->createGPUResource_renderThread(cmdList);

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
		delete reflectionCubemap;
		delete ambientCubemap;
	}

	void SkyboxPass::renderSkybox(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(Skybox);
		SkyboxProxy* skyboxProxy = scene->skybox;

		renderSkyboxToScreen(cmdList, scene);
		if (skyboxProxy->bLightingDirty) {
			Texture* skyboxTexture = skyboxProxy->texture;

			if (skyboxProxy->bUseCubemapTexture) {
				LightProbeBaker::get().blitCubemap_renderThread(cmdList, skyboxTexture, reflectionCubemap, 0, 0);
			} else {
				renderSkyMaterialToCubemap(cmdList, scene);
			}
			// Copy specular cubemap to ambient cubemap for diffuse SH.
			cmdList.generateTextureMipmap(reflectionCubemap->internal_getGLName());
			int32 copyMip = badger::ctz(reflectionCubemap->getCreateParams().width) - badger::ctz(ambientCubemap->getCreateParams().width);
			LightProbeBaker::get().copyCubemap_renderThread(cmdList, reflectionCubemap, ambientCubemap, copyMip, 0);

			renderSkyDiffuseSH(cmdList);
			renderSkyPreftilerMap(cmdList);
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
			uboData.viewProj            = proj * view;
			uboData.skyboxLOD           = skybox->textureLod;
			uboData.intensityMultiplier = skybox->intensityMultiplier;
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
				cmdList.bindTextureUnit(mtp.binding, mtp.texture->internal_getGLName());
			}
		}

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		skybox->cube->bindPositionOnlyVAO(cmdList);
		skybox->cube->drawPrimitive(cmdList);

		cmdList.cullFace(GL_BACK);
	}

	void SkyboxPass::renderSkyMaterialToCubemap(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(SkyMaterialToCubemap);

		GLuint programName = scene->skybox->skyboxMaterial->internal_getMaterialShader()->program->getGLName();
		cmdList.useProgram(programName);

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fboCube);

		const TextureCreateParams& cubeDesc = reflectionCubemap->getCreateParams();
		const GLuint cubeName = reflectionCubemap->internal_getGLName();

		cmdList.viewport(0, 0, cubeDesc.width, cubeDesc.width);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cubeGeometry->bindPositionOnlyVAO(cmdList);

		for (int32 i = 0; i < 6; ++i) {
			cmdList.namedFramebufferTextureLayer(fboCube, GL_COLOR_ATTACHMENT0, cubeName, 0, i);

			// Hack UBO_PerObject
			Material::UBO_PerObject uboData{ cubeTransforms[i], cubeTransforms[i] };
			uboPerObject.update(cmdList, Material::UBO_PerObject::BINDING_POINT, &uboData);

			cubeGeometry->drawPrimitive(cmdList);
		}

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void SkyboxPass::renderSkyDiffuseSH(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(SkyboxToDiffuseSH);
		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		LightProbeBaker::get().bakeDiffuseSH_renderThread(cmdList, ambientCubemap, sceneContext.skyDiffuseSH);
	}

	void SkyboxPass::renderSkyPreftilerMap(RenderCommandList& cmdList) {
		SCOPED_DRAW_EVENT(SkyboxToPrefilterMap);

		constexpr uint32 targetCubemapSize = pathos::SKY_PREFILTER_MAP_DEFAULT_SIZE;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;
		sceneContext.reallocSkyPrefilterMap(cmdList, targetCubemapSize);

		LightProbeBaker::get().bakeSpecularIBL_renderThread(
			cmdList,
			reflectionCubemap->internal_getGLName(),
			targetCubemapSize,
			sceneContext.skyPrefilterMapMipCount,
			sceneContext.skyPrefilteredMap);
	}

}
