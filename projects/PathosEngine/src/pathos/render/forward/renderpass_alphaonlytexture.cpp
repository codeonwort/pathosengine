#include "renderpass_alphaonlytexture.h"
#include "pathos/light/shadow.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

namespace pathos {

	static constexpr unsigned int TEXTURE_UNIT = 0;

	AlphaOnlyTexturePass::AlphaOnlyTexturePass() {
		createProgram();
	}

	void AlphaOnlyTexturePass::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("forward_alphaonly_texture_vs.glsl");
		fs.loadSource("forward_alphaonly_texture_fs.glsl");

		program = pathos::createProgram(vs, fs);

#define GET_UNIFORM(z) { uniform_##z = glGetUniformLocation(program, #z); assert(uniform_##z != -1); }
		GET_UNIFORM(mvpTransform);
		GET_UNIFORM(texSampler);
		GET_UNIFORM(color);
#undef GET_UNIFORM
	}

	void AlphaOnlyTexturePass::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		AlphaOnlyTextureMaterial* material = static_cast<AlphaOnlyTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_uv();
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// texture
		glBindTextureUnit(TEXTURE_UNIT, material->getTexture());

		// uniform
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		glUniform3fv(uniform_color, 1, material->getColor());

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		geometry->draw();

		glDisable(GL_BLEND);
	}

}
