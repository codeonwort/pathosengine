#include "skybox.h"
#include "pathos/scene/scene.h"
#include "pathos/camera/camera.h"
#include "pathos/shader/shader.h"
#include "pathos/util/math_lib.h"

#include "badger/types/vector_types.h"
#include "badger/types/matrix_types.h"
#include <string>

namespace pathos {

	/**
	* @param	inTextureID		See ::loadCubemapTexture() in <pathos/loader/imageloader.h>
	*/
	void Skybox::initialize(GLuint inTextureID) {
		textureID = inTextureID;
		createShader();
		cube = new CubeGeometry(vector3(1.0f));
		lod = 0.0f;
	}

	void Skybox::setLOD(float inLOD) {
		lod = pathos::max(0.0f, inLOD);
	}

	void Skybox::createShader() {
		std::string vshader = R"(#version 430 core

layout (location = 0) in vec3 position;

layout (location = 0) uniform mat4 viewProj;

out VS_OUT { vec3 tc; } vs_out;

void main() {
	vs_out.tc = position;
	gl_Position = (viewProj * vec4(position, 1)).xyww;
}
)";

		std::string fshader = R"(#version 430 core
layout (binding = 0) uniform samplerCube texCube;

layout (location = 1) uniform float skyTextureLOD;

in VS_OUT { vec3 tc; } fs_in;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_bright;

void main() {
  out_color = textureLod(texCube, fs_in.tc, skyTextureLOD);
  out_bright = vec4(0.0);
}
)";
		program = createProgram(vshader, fshader, "Skybox");
		uniform_transform = glGetUniformLocation(program, "viewProj");
		uniform_lod = glGetUniformLocation(program, "skyTextureLOD");
	}

	void Skybox::render(RenderCommandList& cmdList, const Scene* scene, const Camera* camera) {
		SCOPED_DRAW_EVENT(Skybox);

		matrix4 view = matrix4(matrix3(camera->getViewMatrix())); // view transform without transition
		matrix4 proj = camera->getProjectionMatrix();
		matrix4 transform = proj * view;

		cmdList.depthFunc(GL_GEQUAL);
		cmdList.disable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_FRONT);

		cmdList.useProgram(program);
		cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &transform[0][0]);
		cmdList.uniform1f(uniform_lod, lod);
		cmdList.bindTextureUnit(0, textureID);

		cube->activate_position(cmdList);
		cube->activateIndexBuffer(cmdList);

		cube->drawPrimitive(cmdList);
		
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.cullFace(GL_BACK);
	}

	void Skybox::onDestroy() {
		if (program != 0) {
			glDeleteProgram(program);
			program = 0;
		}
		if (cube) {
			delete cube;
			cube = nullptr;
		}
	}

}
