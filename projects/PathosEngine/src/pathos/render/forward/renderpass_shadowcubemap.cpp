#include "renderpass_shadowcubemap.h"
#include "pathos/light/shadow.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

#ifdef _DEBUG
#include <iostream>
#endif

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

#if defined(_DEBUG) && false
		std::cout << "=== ShadowCubeTexturePass ===" << std::endl;
		std::cout << vsSource.getCode() << std::endl;
		std::cout << fsSource.getCode() << std::endl;
#endif

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode());
		positionLocation = vsSource.getPositionLocation();
		uvLocation = vsSource.getUVLocation();
	}

	void ShadowCubeTexturePass::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		ShadowCubeTextureMaterial* material = static_cast<ShadowCubeTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activatePositionBuffer(positionLocation);
		geometry->activateUVBuffer(uvLocation);
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// texture and sampler
		glUniform1i(glGetUniformLocation(program, "texSampler"), ShadowCubeTexturePass::TEXTURE_UNIT);
		glActiveTexture(GL_TEXTURE0 + ShadowCubeTexturePass::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_CUBE_MAP, material->getTexture());
		GLint compareMode, depthTextureMode;
		glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, &compareMode);
		glGetTexParameteriv(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, &depthTextureMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

		// upload uniform
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		glUniform1ui(glGetUniformLocation(program, "face"), material->getFace());
		glUniform1f(glGetUniformLocation(program, "zNear"), material->getZNear());
		glUniform1f(glGetUniformLocation(program, "zFar"), material->getZFar());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivatePositionBuffer(positionLocation);
		geometry->deactivateUVBuffer(uvLocation);
		geometry->deactivateIndexBuffer();
		// restore depth texture params
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, compareMode);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, depthTextureMode);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glUseProgram(0);
	}

}