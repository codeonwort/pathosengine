#include "renderpass_shadowcubemap.h"
#include "pathos/light/shadow.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

namespace pathos {

	ShadowCubeTexturePass::ShadowCubeTexturePass() {
		createProgram();
	}

	void ShadowCubeTexturePass::createProgram() {
		vsSource.setUseUV(true);
		vsSource.setUVLocation(1);

		fsSource.inVar("vec2", "uv");
		fsSource.outVar("vec4", "color");
		fsSource.uniform("samplerCube", "texSampler");
		fsSource.uniform("uint", "face");
		fsSource.uniform("float", "zNear");
		fsSource.uniform("float", "zFar");

		fsSource.mainCode("vec2 uv = fs_in.uv * 2 - vec2(1.0);");
		fsSource.mainCode("float depth;");
		fsSource.mainCode("if (face == 0) depth = texture(texSampler, vec3(1, uv.y, uv.x)).r;");
		fsSource.mainCode("else if (face == 1) depth = texture(texSampler, vec3(-1, uv.y, uv.x)).r;");
		fsSource.mainCode("else if (face == 2) depth = texture(texSampler, vec3(uv.y, 1, uv.x)).r;");
		fsSource.mainCode("else if (face == 3) depth = texture(texSampler, vec3(uv.y, -1, uv.x)).r;");
		fsSource.mainCode("else if (face == 4) depth = texture(texSampler, vec3(uv.x, uv.y, 1)).r;");
		fsSource.mainCode("else depth = texture(texSampler, vec3(uv.x, uv.y, -1)).r;");
		fsSource.mainCode("depth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));");
		fsSource.mainCode("color = vec4(depth, depth, depth, 1.0);");

		dumpShaderSource(vsSource, "renderpass_shadowcubemap.vert");
		dumpShaderSource(fsSource, "renderpass_shadowcubemap.frag");

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode());

		uniform_mvpTransform   = glGetUniformLocation(program, "mvpTransform");
		uniform_face           = glGetUniformLocation(program, "face");
		uniform_zNear          = glGetUniformLocation(program, "zNear");
		uniform_zFar           = glGetUniformLocation(program, "zFar");
		assert(uniform_mvpTransform != -1);
		assert(uniform_face != -1);
		assert(uniform_zNear != -1);
		assert(uniform_zFar != -1);
	}

	void ShadowCubeTexturePass::render(Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		static_cast<void>(scene);
		ShadowCubeTextureMaterial* material = static_cast<ShadowCubeTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_uv();
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// texture and sampler
		glActiveTexture(GL_TEXTURE0 + ShadowCubeTexturePass::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_CUBE_MAP, material->getTexture());
		GLint compareMode;
		glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, &compareMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		// upload uniform
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		glUniform1ui(uniform_face, material->getFace());
		glUniform1f(uniform_zNear, material->getZNear());
		glUniform1f(uniform_zFar, material->getZFar());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivate();
		geometry->deactivateIndexBuffer();
		// restore depth texture params
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, compareMode);
		glUseProgram(0);
	}

}
