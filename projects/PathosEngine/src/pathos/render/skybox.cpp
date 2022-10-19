#include "skybox.h"
#include "pathos/engine_policy.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/camera/camera.h"
#include "pathos/util/math_lib.h"
#include "pathos/shader/shader_program.h"

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
		matrix4 viewProj;
		float skyboxLOD;
	};

	DEFINE_SHADER_PROGRAM2(Program_Skybox, SkyboxVS, SkyboxFS);

}

namespace pathos {

	void SkyboxPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_Skybox");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		ubo.init<UBO_Skybox>();
	}

	void SkyboxPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void SkyboxPass::render(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(Skybox);

		const Camera& camera = scene->camera;
		SkyboxProxy* skybox = scene->skybox;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Skybox);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		UBO_Skybox uboData;
		{
			matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
			matrix4 proj = camera.getProjectionMatrix();
			uboData.viewProj = proj * view;
			uboData.skyboxLOD = skybox->textureLod;
		}
		ubo.update(cmdList, 1, &uboData);

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		cmdList.cullFace(GL_FRONT);

		cmdList.bindTextureUnit(0, skybox->textureID);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		skybox->cube->activate_position(cmdList);
		skybox->cube->activateIndexBuffer(cmdList);
		skybox->cube->drawPrimitive(cmdList);

		cmdList.cullFace(GL_BACK);
	}

}

namespace pathos {

	SkyboxComponent::~SkyboxComponent() {
		if (cube) {
			delete cube;
			cube = nullptr;
		}
	}

	void SkyboxComponent::initialize(GLuint inTextureID) {
		textureID = inTextureID;
		lod = 0.0f;
		cube = new CubeGeometry(vector3(1.0f));
	}

	void SkyboxComponent::setLOD(float inLOD) {
		lod = pathos::max(0.0f, inLOD);
	}

	void SkyboxComponent::createRenderProxy(SceneProxy* scene) {
		if (!hasValidResources()) {
			scene->skybox = nullptr;
			return;
		}

		SkyboxProxy* proxy = ALLOC_RENDER_PROXY<SkyboxProxy>(scene);
		proxy->cube = cube;
		proxy->textureID = textureID;
		proxy->textureLod = lod;

		scene->skybox = proxy;
	}

}

namespace pathos {

	/**
	* @param	inTextureID		See ::loadCubemapTexture() in <pathos/loader/imageloader.h>
	*/
	void Skybox::initialize(GLuint inTextureID) {
		component->initialize(inTextureID);
	}

	void Skybox::setLOD(float inLOD) {
		component->setLOD(inLOD);
	}

}
