#include "renderpass_shadowtexture.h"
#include "pathos/light/shadow.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

namespace pathos {

	ShadowTexturePass::ShadowTexturePass() {
		createProgram();
	}

	void ShadowTexturePass::createProgram() {
		vsSource.setUseUV(true);
		vsSource.setUVLocation(1);

		fsSource.inVar("vec2", "uv");
		fsSource.outVar("vec4", "color");
		fsSource.textureSampler("texSampler");

		fsSource.mainCode("float depth = texture2D(texSampler, fs_in.uv).r;");
		// do we need clamp?
		//fsSource.mainCode("depth = clamp(depth, 0.0f, 1.0f);");
		fsSource.mainCode("color = vec4(depth, depth, depth, 1.0);");

		dumpShaderSource(vsSource, "renderpass_shadowtexture.vert");
		dumpShaderSource(fsSource, "renderpass_shadowtexture.frag");
		
		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode(), "ForwardPass_ShadowTexture");
	}

	void ShadowTexturePass::renderMeshPass(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		static_cast<void>(scene);
		ShadowTextureMaterial* material = static_cast<ShadowTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_uv(cmdList);
		geometry->activateIndexBuffer(cmdList);

		glUseProgram(program);

		// texture and sampler
		glUniform1i(glGetUniformLocation(program, "texSampler"), ShadowTexturePass::TEXTURE_UNIT);
		glActiveTexture(GL_TEXTURE0 + ShadowTexturePass::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getTexture());
		GLint compareMode;
		glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, &compareMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

		// upload uniform
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->drawPrimitive(cmdList);

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivate(cmdList);
		geometry->deactivateIndexBuffer(cmdList);
		// restore depth texture params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, compareMode);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}

}
