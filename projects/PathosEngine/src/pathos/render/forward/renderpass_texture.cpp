#include "renderpass_texture.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	FlatTexturePass::FlatTexturePass(unsigned int maxDirLights, unsigned int maxPointLights)
		:maxDirectionalLights(maxDirLights), maxPointLights(maxPointLights)
	{
		createProgram();
	}

	void FlatTexturePass::createProgram() {
		vsSource.setUseUV(true);
		vsSource.setUVLocation(1);
		fsSource.inVar("vec2", "uv");
		fsSource.outVar("vec4", "color");

		fsSource.textureSampler("texSampler");
		fsSource.uniform("vec3", "specularColor");
		fsSource.uniform("uint", "numDirLights");
		fsSource.uniform("uint", "numPointLights");
		fsSource.uniform("bool", "useLighting");

		// light
		vsSource.setUseNormal(true);
		vsSource.setTransferPosition(true);
		fsSource.uniform("vec3", "eye");
		fsSource.inVar("vec3", "normal");
		fsSource.inVar("vec3", "position");
		fsSource.directionalLights(maxDirectionalLights);
		fsSource.pointLights(maxPointLights);

		// shadow
		fsSource.mainCode("float visibility = 1.0;");
		ShadowMap::addShaderCode(vsSource, fsSource, maxDirectionalLights);
		OmnidirectionalShadow::addShaderCode(vsSource, fsSource, maxPointLights);

		fsSource.mainCode("vec3 diffuseLightAccum;");
		fsSource.mainCode("vec3 specularLightAccum;");
		fsSource.mainCode("if (useLighting) {");
		fsSource.mainCode("  diffuseLightAccum = vec3(0.0f);");
		fsSource.mainCode("  specularLightAccum = vec3(0.0f);");
		fsSource.mainCode("} else {");
		fsSource.mainCode("  diffuseLightAccum = vec3(1.0f);");
		fsSource.mainCode("  specularLightAccum = vec3(0.0f);");
		fsSource.mainCode("}");

		fsSource.mainCode("if (useLighting) {");
			// directional lighting
			fsSource.mainCode("vec3 norm = normalize(fs_in.normal);");
			fsSource.mainCode("vec3 halfVector;");
			fsSource.mainCode("for(unsigned int i = 0; i < numDirLights; ++i){");
			fsSource.mainCode("  vec3 lightDir = -dirLightDirs[i];");
			fsSource.mainCode("  vec3 lightCol = dirLightColors[i];");
			fsSource.mainCode("  diffuseLightAccum += lightCol * max(dot(norm, lightDir), 0.0f);");
			fsSource.mainCode("  halfVector = normalize(lightDir + eye);");
			fsSource.mainCode("  specularLightAccum += pow(max(dot(norm, halfVector), 0.0f), 128);");
			fsSource.mainCode("}");
			
			// point lighting
			fsSource.mainCode("vec3 norm2 = normalize(fs_in.normal);"); // normal in world space
			fsSource.mainCode("vec3 halfVector2;");
			fsSource.mainCode("for(unsigned int i = 0; i < numPointLights; ++i){");
			fsSource.mainCode("  vec3 lightDir = normalize(pointLightPos[i] - fs_in.position);");
			fsSource.mainCode("  vec3 lightCol = pointLightColors[i];");
			fsSource.mainCode("  diffuseLightAccum += lightCol * max(dot(norm2, lightDir), 0.0f);");
			fsSource.mainCode("  halfVector2 = normalize(lightDir + eye);");
			fsSource.mainCode("  specularLightAccum += pow(max(dot(norm2, halfVector2), 0.0f), 128);");
			fsSource.mainCode("}");
		fsSource.mainCode("}");

		fsSource.mainCode("color = texture2D(texSampler, fs_in.uv);");
		fsSource.mainCode("color.rgb *= visibility;");
		fsSource.mainCode("if (useLighting) {");
		fsSource.mainCode("  color.rgb = color.rgb * diffuseLightAccum + visibility * specularColor * specularLightAccum;");
		fsSource.mainCode("}");
		//fsSource.mainCode("if (color.a <= 0.0f) discard;");

		dumpShaderSource(vsSource, "renderpass_texture.vert");
		dumpShaderSource(fsSource, "renderpass_texture.frag");

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode(), "ForwardPass_Texture");
	}

	void FlatTexturePass::renderMeshPass(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		TextureMaterial* material = static_cast<TextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_uv_normal(cmdList);
		geometry->activateIndexBuffer(cmdList);

		glUseProgram(program);

		// texture
		glUniform1i(glGetUniformLocation(program, "texSampler"), FlatTexturePass::DIFFUSE_TEXTURE_UNIT);
		glActiveTexture(GL_TEXTURE0 + FlatTexturePass::DIFFUSE_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		glUniform3fv(glGetUniformLocation(program, "specularColor"), 1, material->getSpecular());
		glm::vec3 eye = camera->getEyeVector();
		glUniform3f(glGetUniformLocation(program, "eye"), -eye.x, -eye.y, -eye.z);

		// upload light
		uploadDirectionalLightUniform(scene, maxDirectionalLights);
		uploadPointLightUniform(scene, maxPointLights);
		if (shadowMapping != nullptr) {
			shadowMapping->activate(program, scene->proxyList_directionalLight, SHADOW_MAPPING_TEXTURE_UNIT_START, modelMatrix);
		}
		if (omniShadow != nullptr) {
			omniShadow->activate(program, scene->proxyList_pointLight, OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START, modelMatrix);
		}
		glUniform1ui(glGetUniformLocation(program, "useLighting"), material->getLighting());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		if (material->getUseAlpha()) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		geometry->drawPrimitive(cmdList);
		glDisable(GL_BLEND);

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivate(cmdList);
		geometry->deactivateIndexBuffer(cmdList);

		if (shadowMapping != nullptr) {
			shadowMapping->deactivate(program, SHADOW_MAPPING_TEXTURE_UNIT_START);
		}
		if (omniShadow != nullptr) {
			omniShadow->deactivate(program, OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START);
		}

		glUseProgram(0);
	}

}