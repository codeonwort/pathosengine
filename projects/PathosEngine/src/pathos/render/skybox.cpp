#include "skybox.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/util/math_lib.h"
#include "pathos/shader/shader_program.h"

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include <string>

namespace pathos {
	
	class SkyboxVS : public ShaderStage {
	public:
		SkyboxVS() : ShaderStage(GL_VERTEX_SHADER, "SkyboxVS") {
			addDefine("VERTEX_SHADER 1");
			setFilepath("skybox.glsl");
		}
	};

	class SkyboxFS : public ShaderStage {
	public:
		SkyboxFS() : ShaderStage(GL_FRAGMENT_SHADER, "SkyboxFS") {
			addDefine("FRAGMENT_SHADER 1");
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
		ubo.init<UBO_Skybox>();
	}

	void SkyboxPass::destroyResources(RenderCommandList& cmdList) {
		//
	}

	void SkyboxPass::render(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(Skybox);

		const Camera& camera = scene->camera;
		SkyboxProxy* skybox = scene->skybox;

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_Skybox);
		cmdList.useProgram(program.getGLName());

		UBO_Skybox uboData;
		{
			matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
			matrix4 proj = camera.getProjectionMatrix();
			uboData.viewProj = proj * view;
			uboData.skyboxLOD = skybox->textureLod;
		}
		ubo.update(cmdList, 1, &uboData);

		cmdList.depthFunc(GL_GEQUAL);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cmdList.bindTextureUnit(0, skybox->textureID);

		skybox->cube->activate_position(cmdList);
		skybox->cube->activateIndexBuffer(cmdList);
		skybox->cube->drawPrimitive(cmdList);

		cmdList.enable(GL_DEPTH_TEST);
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
