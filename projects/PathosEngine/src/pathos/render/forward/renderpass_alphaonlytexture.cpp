#include "renderpass_alphaonlytexture.h"
#include "pathos/light/shadow.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

//#define DEBUG_ALPHAONLYTEXTURE

#if defined(_DEBUG) && defined(DEBUG_ALPHAONLYTEXTURE)
#include <iostream>
#endif

namespace pathos {

	AlphaOnlyTexturePass::AlphaOnlyTexturePass() {
		createProgram();
	}

	void AlphaOnlyTexturePass::createProgram() {
		vsSource.setUseUV(true);
		vsSource.setUVLocation(1);

		fsSource.uniform("vec3", "color");
		fsSource.inVar("vec2", "uv");
		fsSource.outVar("vec4", "out_color");
		fsSource.textureSampler("texSampler");

		fsSource.mainCode("float alpha = texture2D(texSampler, fs_in.uv).r;");
		//fsSource.mainCode("out_color = vec4(vec3((cos(gl_FragCoord.x / 30) + 1.0f) / 2.0f), alpha);");
		fsSource.mainCode("out_color = vec4(color, alpha);");
		//fsSource.mainCode("out_color = vec4(alpha, alpha, alpha, alpha);");

#if defined(_DEBUG) && false
		std::cout << "=== AlphaOnlyTexturePass ===" << std::endl;
		std::cout << vsSource.getCode() << std::endl;
		std::cout << fsSource.getCode() << std::endl;
#endif

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode());
		positionLocation = vsSource.getPositionLocation();
		uvLocation = vsSource.getUVLocation();
	}

	void AlphaOnlyTexturePass::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		AlphaOnlyTextureMaterial* material = static_cast<AlphaOnlyTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activatePositionBuffer(positionLocation);
		geometry->activateUVBuffer(uvLocation);
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// texture and sampler
		glUniform1i(glGetUniformLocation(program, "texSampler"), AlphaOnlyTexturePass::TEXTURE_UNIT);
		glActiveTexture(GL_TEXTURE0 + AlphaOnlyTexturePass::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getTexture());

		// upload uniform
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		glUniform3fv(glGetUniformLocation(program, "color"), 1, material->getColor());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		geometry->draw();
		glDisable(GL_BLEND);

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivatePositionBuffer(positionLocation);
		geometry->deactivateUVBuffer(uvLocation);
		geometry->deactivateIndexBuffer();
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}

}