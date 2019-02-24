#include "renderpass_bumptexture.h"
#include "glm/gtc/type_ptr.hpp"

#define DEBUG_BUMP_MAPPING 0

#if (_DEBUG && DEBUG_BUMP_MAPPING)
#include <iostream>
using namespace std;
#endif

namespace pathos {

	BumpTexturePass::BumpTexturePass(unsigned int maxDirLights, unsigned int maxPointLights)
		:maxDirectionalLights(maxDirLights), maxPointLights(maxPointLights)
	{
		createProgram();
	}

	void BumpTexturePass::createProgram() {
		vsSource.setUseUV(true);
		vsSource.setUseNormal(true);
		vsSource.setUseTangent(true);
		vsSource.setUseBitangent(true);

		vsSource.uniform("vec3", "eyeDir_camera");
		vsSource.uniform("vec3", "lightPos_camera");
		vsSource.uniform("mat3", "mvTransform3x3");
		vsSource.uniform("mat4", "mvTransform4x4");

		vsSource.outVar("vec3", "light_tangent");
		vsSource.outVar("vec3", "eye_tangent");
		vsSource.outVar("float", "light_distance");

		vsSource.mainCode("vec3 tangent_camera = mvTransform3x3 * tangent;");
		vsSource.mainCode("vec3 bitangent_camera = mvTransform3x3 * bitangent;");
		vsSource.mainCode("vec3 normal_camera = mvTransform3x3 * normal;");
		vsSource.mainCode("mat3 TBN = transpose(mat3(tangent_camera, bitangent_camera, normal_camera));");
		vsSource.mainCode("vec3 position_camera = vec3(mvTransform4x4 * vec4(position, 1.0f));");
		vsSource.mainCode("vec3 lightDir = lightPos_camera - position_camera;");
		vsSource.mainCode("vs_out.light_distance = dot(lightDir, lightDir);");
		vsSource.mainCode("vs_out.light_tangent = TBN * normalize(lightDir);");
		vsSource.mainCode("vs_out.eye_tangent = TBN * eyeDir_camera;");

		fsSource.textureSampler("diffuseSampler");
		fsSource.textureSampler("normalSampler");
		fsSource.inVar("vec2", "uv");
		fsSource.inVar("vec3", "normal");
		fsSource.inVar("vec3", "light_tangent");
		fsSource.inVar("vec3", "eye_tangent");
		fsSource.inVar("float", "light_distance");
		fsSource.outVar("vec4", "color");

		// light
		fsSource.uniform("uint", "numDirLights");
		fsSource.uniform("uint", "numPointLights");
		fsSource.directionalLights(maxDirectionalLights);
		fsSource.pointLights(maxPointLights);

		// shadow
		fsSource.mainCode("float visibility = 1.0f;");
		ShadowMap::addShaderCode(vsSource, fsSource, maxDirectionalLights);
		OmnidirectionalShadow::addShaderCode(vsSource, fsSource, maxPointLights);

		// normal mapping
		fsSource.mainCode("vec3 norm = normalize(texture2D(normalSampler, fs_in.uv).rgb * 2.0f - 1.0f);");

		// directional lighting
		fsSource.mainCode("vec3 diffuseTerm = vec3(0.0f);");
		fsSource.mainCode("vec3 specularTerm = vec3(0.0f);");

		fsSource.mainCode("vec3 halfVector;");
		fsSource.mainCode("halfVector = normalize(fs_in.light_tangent + fs_in.eye_tangent);");
		fsSource.mainCode("float lambert = clamp(dot(norm, normalize(fs_in.light_tangent)), 0.0, 1.0);");
		fsSource.mainCode("float attenuation = 1 / (1 + 0.0003 * fs_in.light_distance);");
		fsSource.mainCode("diffuseTerm = attenuation * visibility * vec3(1.0f) * lambert;");
		fsSource.mainCode("specularTerm = visibility * vec3(1.0f) * pow(max(dot(norm, halfVector), 0.0f), 128);");

		//if (dirLights > 0) {
		//	fsSource.mainCode("vec3 diffuseLightAccum = vec3(0, 0, 0);");
		//	fsSource.mainCode("vec3 specularLightAccum = vec3(0, 0, 0);");
		//	fsSource.mainCode("vec3 halfVector;");
		//	for (size_t i = 0; i < dirLights; i++) {
		//		string lightCol = "dirLightColors[" + to_string(i) + "]";
		//		string lightDir = "dirLightDirs[" + to_string(i) + "]";
		//		fsSource.mainCode("  diffuseLightAccum += " + lightCol + " * max(dot(norm," + lightDir + "),0);");
		//		fsSource.mainCode("halfVector = normalize(" + lightDir + " + fs_in.eye_camera);");
		//		fsSource.mainCode("  specularLightAccum += " + lightCol + " * pow(max(dot(norm,halfVector),0), 128);");
		//	}
		//	fsSource.mainCode("diffuseTerm = visibility * diffuseLightAccum;");
		//	fsSource.mainCode("specularTerm = visibility * specularLightAccum;");
		//}

		//if (useAlpha) fsSource.mainCode("color = vec3(ir, ig, ib) * texture2D(diffuseSampler, fs_in.uv).rgb;");
		//else fsSource.mainCode("color = vec4(ir, ig, ib, 1) * texture2D(diffuseSampler, fs_in.uv);");
		//if (useAlpha) {
			//fsSource.mainCode("color = specularTerm + diffuseTerm * texture2D(diffuseSampler, fs_in.uv).rgb;");
		//} else {
		fsSource.mainCode("vec4 final_diffuse = vec4(diffuseTerm, 1.0) * texture2D(diffuseSampler, fs_in.uv);");
		fsSource.mainCode("vec4 final_specular = 0.8 * vec4(specularTerm, 1.0);");
		fsSource.mainCode("color = visibility * (final_diffuse + final_specular);");
		//}

#if (_DEBUG && DEBUG_BUMP_MAPPING)
		cout << "[" << __FILE__ << "]" << endl;
		cout << "[Vertex Shader]" << endl << vsSource.getCode() << endl << endl;
		cout << "[Fragment Shader]" << endl << fsSource.getCode() << endl << endl;
#endif

		dumpShaderSource(vsSource, "renderpass_bumptexture.vert");
		dumpShaderSource(fsSource, "renderpass_bumptexture.frag");

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode());
	}

	void BumpTexturePass::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		BumpTextureMaterial* material = static_cast<BumpTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activate_position_uv_normal_tangent_bitangent();
		geometry->activateIndexBuffer();

		PointLight* light = scene->pointLights[0];

		glUseProgram(program);

		// uniform: matrix
		const glm::mat4& modelTransform = modelMatrix;
		const glm::mat4 viewTransform = camera->getViewMatrix();
		const glm::mat4 mvTransform4x4 = viewTransform * modelTransform;
		const glm::mat3 mvTransform3x3 = glm::mat3(viewTransform * modelTransform);
		const glm::mat4& mvpTransform = camera->getViewProjectionMatrix() * modelTransform;
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(mvpTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvTransform4x4"), 1, false, glm::value_ptr(mvTransform4x4));
		glUniformMatrix3fv(glGetUniformLocation(program, "mvTransform3x3"), 1, false, glm::value_ptr(mvTransform3x3));
		
		// uniform: vector
		glm::vec3 light_cameraspace = glm::vec3(viewTransform * glm::vec4(light->getPosition(), 1.0f));
		glm::vec3 eye_cameraspace = glm::mat3(viewTransform) * camera->getEyeVector();
		glUniform3f(glGetUniformLocation(program, "lightPos_camera"), light_cameraspace.x, light_cameraspace.y, light_cameraspace.z);
		glUniform3f(glGetUniformLocation(program, "eyeDir_camera"), -eye_cameraspace.x, -eye_cameraspace.y, -eye_cameraspace.z);
		
		// uniform: sampler
		glUniform1i(glGetUniformLocation(program, "diffuseSampler"), BumpTexturePass::DIFFUSE_TEXTURE_UNIT);
		glUniform1i(glGetUniformLocation(program, "normalSampler"), BumpTexturePass::NORMALMAP_TEXTURE_UNIT);
		
		// diffuse texture binding
		glActiveTexture(GL_TEXTURE0 + BumpTexturePass::DIFFUSE_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getDiffuseTexture());
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		// normal map texture binding
		glActiveTexture(GL_TEXTURE0 + BumpTexturePass::NORMALMAP_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getNormalMapTexture());
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// uniform: light
		uploadDirectionalLightUniform(scene, maxDirectionalLights);
		uploadPointLightUniform(scene, maxPointLights);
		if (shadowMapping != nullptr) {
			shadowMapping->activate(program, scene->directionalLights, SHADOW_MAPPING_TEXTURE_UNIT_START, modelMatrix);
		}
		if (omniShadow != nullptr) {
			omniShadow->activate(program, scene->pointLights, OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START, modelMatrix);
		}

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------

		if (shadowMapping != nullptr) {
			shadowMapping->deactivate(program, SHADOW_MAPPING_TEXTURE_UNIT_START);
		}
		if (omniShadow != nullptr) {
			omniShadow->deactivate(program, OMNIDIRECTIONAL_SHADOW_TEXTURE_UNIT_START);
		}

		glUseProgram(0);
	}

}