#include "renderpass_solidcolor.h"
#include "pathos/light/shadow.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

namespace pathos {

	SolidColorPass::SolidColorPass(unsigned int maxDirLights, unsigned int maxPointLights)
		:maxDirectionalLights(maxDirLights), maxPointLights(maxPointLights)
	{
		createProgram();
	}

	void SolidColorPass::createProgram() {
		// light
		vsSource.setUseNormal(true);
		vsSource.setTransferPosition(true);

		fsSource.inVar("vec3", "position");
		fsSource.inVar("vec3", "normal");
		fsSource.directionalLights(maxDirectionalLights);
		fsSource.pointLights(maxPointLights);
		fsSource.uniform("vec3", "ambientColor");
		fsSource.uniform("vec3", "diffuseColor");
		fsSource.uniform("vec3", "specularColor");
		fsSource.uniform("vec3", "eye");
		fsSource.uniform("float", "materialAlpha");
		fsSource.uniform("uint", "numDirLights");
		fsSource.uniform("uint", "numPointLights");
		fsSource.outVar("vec4", "outColor");

		// shadow
		fsSource.mainCode("float visibility = 1.0;");
		ShadowMap::addShaderCode(vsSource, fsSource, maxDirectionalLights);
		OmnidirectionalShadow::addShaderCode(vsSource, fsSource, maxPointLights);

		// directional lighting
		fsSource.mainCode("vec3 diffuseTerm = vec3(0, 0, 0);");
		fsSource.mainCode("vec3 specularTerm = vec3(0, 0, 0);");
		fsSource.mainCode("vec3 norm = normalize(fs_in.normal);"); // normal in world space
		
		fsSource.mainCode("vec3 diffuseLightAccum = vec3(0, 0, 0);");
		fsSource.mainCode("vec3 specularLightAccum = vec3(0, 0, 0);");
		fsSource.mainCode("vec3 halfVector;");

		fsSource.mainCode("for(unsigned int i = 0; i < numDirLights; ++i){");
		fsSource.mainCode("  vec3 lightDir = -dirLightDirs[i];");
		fsSource.mainCode("  vec3 lightCol = dirLightColors[i];");
		fsSource.mainCode("  diffuseLightAccum += lightCol * max(dot(norm, lightDir), 0.0f);");
		// blinn-phong shading
		fsSource.mainCode("  halfVector = normalize(lightDir + eye);");
		fsSource.mainCode("  specularLightAccum += pow(max(dot(norm, halfVector), 0.0f), 128);");
		// original phong shading
		//fsSource.mainCode("  halfVector = reflect(lightDir, norm);");
		//fsSource.mainCode("  specularLightAccum += specularColor * pow(max(dot(halfVector, -eye), 0.0f), 128);");
		fsSource.mainCode("}");

		fsSource.mainCode("diffuseTerm = visibility * diffuseColor * diffuseLightAccum;");
		fsSource.mainCode("specularTerm = visibility * specularColor * specularLightAccum;");
		
		// point lighting
		fsSource.mainCode("vec3 diffuseLightAccum2 = vec3(0, 0, 0);");
		fsSource.mainCode("vec3 specularLightAccum2 = vec3(0, 0, 0);");
		fsSource.mainCode("vec3 halfVector2;");

		fsSource.mainCode("for(unsigned int i = 0; i < numPointLights; ++i){");
		fsSource.mainCode("  vec3 lightDir = normalize(pointLightPos[i] - fs_in.position);");
		fsSource.mainCode("  vec3 lightCol = pointLightColors[i];");
		fsSource.mainCode("  diffuseLightAccum2 += lightCol * max(dot(norm, lightDir), 0.0f);");
		fsSource.mainCode("  halfVector2 = normalize(lightDir + eye);");
		fsSource.mainCode("  specularLightAccum2 += pow(max(dot(norm, halfVector2), 0.0f), 128);");
		fsSource.mainCode("}");

		fsSource.mainCode("diffuseTerm += visibility * diffuseColor * diffuseLightAccum2;");
		fsSource.mainCode("specularTerm += visibility * specularColor * specularLightAccum2;");

		// rim lighting
		fsSource.mainCode("const float rimPower = 3.2;");
		//fsSource.mainCode("vec3 rim = visibility * vec3(1.0) * pow(1.0f - max(dot(norm, eye), 0.0f), rimPower);");
		fsSource.mainCode("vec3 rim = vec3(0,0,0);");

		// final shading
		//fsSource.mainCode("diffuseTerm.r = ceil(diffuseTerm.r*5)/5.0;");
		//fsSource.mainCode("diffuseTerm.g = ceil(diffuseTerm.g*5)/5.0;");
		//fsSource.mainCode("diffuseTerm.b = ceil(diffuseTerm.b*5)/5.0;");
		fsSource.mainCode("outColor.rgb = rim + ambientColor + diffuseTerm + specularTerm;");
		fsSource.mainCode("outColor.a = materialAlpha;");

		dumpShaderSource(vsSource, "renderpass_solidcolor.vert");
		dumpShaderSource(fsSource, "renderpass_solidcolor.frag");

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode(), "ForwardPass_SolidColor");
	}

	void SolidColorPass::renderMeshPass(RenderCommandList& cmdList, Scene* scene, Camera* camera, MeshGeometry* geometry, Material* material_) {
		ColorMaterial* material = static_cast<ColorMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_normal(cmdList);
		geometry->activateIndexBuffer(cmdList);

		glUseProgram(program);

		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(camera->getViewProjectionMatrix() * modelMatrix));
		//glUniform3fv(glGetUniformLocation(program, "ambientColor"), 1, material->getAmbient());
		glUniform3fv(glGetUniformLocation(program, "diffuseColor"), 1, material->getAlbedo());
		//glUniform3fv(glGetUniformLocation(program, "specularColor"), 1, material->getSpecular());
		glm::vec3 eye = camera->getEyeVector();
		glUniform3f(glGetUniformLocation(program, "eye"), -eye.x, -eye.y, -eye.z);

		uploadDirectionalLightUniform(scene, maxDirectionalLights);
		uploadPointLightUniform(scene, maxPointLights);
		if (shadowMapping != nullptr) {
			shadowMapping->activate(program, scene->proxyList_directionalLight, SHADOW_MAPPING_TEXTURE_UNIT_START, modelMatrix);
		}
		if (omniShadow != nullptr) {
			omniShadow->activate(program, scene->proxyList_pointLight, OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START, modelMatrix);
		}

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->drawPrimitive(cmdList);

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
