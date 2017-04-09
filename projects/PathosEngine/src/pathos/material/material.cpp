#include "pathos/material/material.h"

namespace pathos {

	/*
	PlaneReflection* MeshMaterial::getReflectionMethod() { return reflectionMethod; }
	void MeshMaterial::setReflectionMethod(PlaneReflection* pr) {
		reflectionMethod = pr;
		pr->setReflector(owner);
		programDirty = true;
	}
	*/
	
	////////////////////////////////////////////////////////////////////////////////////
	// ColorMaterial
	ColorMaterial::ColorMaterial() {
		materialID = MATERIAL_ID::SOLID_COLOR;
		setAmbient(0.0f, 0.0f, 0.0f);
		setDiffuse(0.5f, 0.5f, 0.5f);
		setSpecular(1.0f, 1.0f, 1.0f);
		setAlpha(1.0f);
		blendSrcFactor = GL_SRC_ALPHA;
		blendDstFactor = GL_ONE_MINUS_SRC_ALPHA;
	}

	void ColorMaterial::setAmbient(GLfloat r, GLfloat g, GLfloat b) { ambient[0] = r; ambient[1] = g; ambient[2] = b; }
	void ColorMaterial::setDiffuse(GLfloat r, GLfloat g, GLfloat b) { diffuse[0] = r; diffuse[1] = g; diffuse[2] = b; }
	void ColorMaterial::setSpecular(GLfloat r, GLfloat g, GLfloat b) { specular[0] = r; specular[1] = g; specular[2] = b; }
	void ColorMaterial::setAlpha(GLfloat a) { alpha = a; }
	void ColorMaterial::setBlendFactor(GLuint srcFactor, GLuint dstFactor) { blendSrcFactor = srcFactor; blendDstFactor = dstFactor; }

	////////////////////////////////////////////////////////////////////////////////////
	// TextureMaterial
	TextureMaterial::TextureMaterial(GLuint diffuseTexture) : texture(diffuseTexture) {
		materialID = MATERIAL_ID::FLAT_TEXTURE;
		setSpecular(1.0f, 1.0f, 1.0f);
	}

	void TextureMaterial::setSpecular(GLfloat r, GLfloat g, GLfloat b) { specular[0] = r; specular[1] = g; specular[2] = b; }

	////////////////////////////////////////////////////////////////////////////////////
	// BumpTextureMaterial
	BumpTextureMaterial::BumpTextureMaterial(GLuint diffuseTexture, GLuint normalMapTexture)
		: diffuseTexture(diffuseTexture), normalMapTexture(normalMapTexture)
	{
		materialID = MATERIAL_ID::BUMP_TEXTURE;
	}
	/*
	// BumpTextureMaterialPass
	BumpTextureMaterialPass::BumpTextureMaterialPass(GLuint imageTexture, GLuint normalMapTexture, PointLight* light, bool useAlpha)
		: imageTexture(imageTexture), normalMapTexture(normalMapTexture), light(light), useAlpha(useAlpha) {
		//
	}
	void BumpTextureMaterialPass::updateProgram(MeshMaterial* M) {
		vsCompiler.setUseUV(true);
		vsCompiler.setUseNormal(true);
		vsCompiler.setUseTangent(true);
		vsCompiler.setUseBitangent(true);

		vsCompiler.uniform("vec3", "eyeDir_camera");
		vsCompiler.uniform("vec3", "lightPos_camera");
		vsCompiler.uniform("mat3", "mvTransform3x3");
		vsCompiler.uniform("mat4", "mvTransform4x4");

		vsCompiler.outVar("vec3", "light_tangent");
		vsCompiler.outVar("vec3", "eye_tangent");
		vsCompiler.outVar("float", "light_distance");

		vsCompiler.mainCode("vec3 tangent_camera = mvTransform3x3 * tangent;");
		vsCompiler.mainCode("vec3 bitangent_camera = mvTransform3x3 * bitangent;");
		vsCompiler.mainCode("vec3 normal_camera = mvTransform3x3 * normal;");
		vsCompiler.mainCode("mat3 TBN = transpose(mat3(tangent_camera, bitangent_camera, normal_camera));");
		vsCompiler.mainCode("vec3 position_camera = vec3(mvTransform4x4 * vec4(position,1));");
		vsCompiler.mainCode("vec3 lightDir = lightPos_camera - position_camera;");
		vsCompiler.mainCode("vs_out.light_distance = dot(lightDir, lightDir);");
		vsCompiler.mainCode("vs_out.light_tangent = TBN * normalize(lightDir);");
		vsCompiler.mainCode("vs_out.eye_tangent = TBN * eyeDir_camera;");

		fsCompiler.textureSampler("imageSampler");
		fsCompiler.textureSampler("normalSampler");
		fsCompiler.inVar("vec2", "uv");
		fsCompiler.inVar("vec3", "normal");
		fsCompiler.inVar("vec3", "light_tangent");
		fsCompiler.inVar("vec3", "eye_tangent");
		fsCompiler.inVar("float", "light_distance");
		fsCompiler.outVar("vec4", "color");

		// light
		size_t dirLights = M->getDirectionalLights().size();
		if (dirLights > 0) {
			//vsCompiler.setUseNormal(true);
			fsCompiler.directionalLights(dirLights);
			fsCompiler.inVar("vec3", "normal");
		}

		// shadow
		fsCompiler.mainCode("float visibility = 1.0;");
		if (M->getShadowMethod() != nullptr) {
			M->getShadowMethod()->addShaderCode(vsCompiler, fsCompiler);
		}

		// normal mapping
		fsCompiler.mainCode("vec3 norm = normalize(texture2D(normalSampler, fs_in.uv).rgb * 2.0 - 1.0);");

		// directional lighting
		fsCompiler.mainCode("vec3 diffuseTerm = vec3(0, 0, 0);");
		fsCompiler.mainCode("vec3 specularTerm = vec3(0, 0, 0);");

		fsCompiler.mainCode("vec3 halfVector;");
		fsCompiler.mainCode("halfVector = normalize(fs_in.light_tangent + fs_in.eye_tangent);");
		fsCompiler.mainCode("float lambert = clamp(dot(norm, normalize(fs_in.light_tangent)), 0.0, 1.0);");
		fsCompiler.mainCode("diffuseTerm = (1/(1+0.001*fs_in.light_distance)) * visibility * vec3(1,1,1) * lambert;");
		fsCompiler.mainCode("specularTerm = visibility * vec3(1,1,1) * pow(max(dot(norm, halfVector),0), 128);");

		//if (dirLights > 0) {
		//	fsCompiler.mainCode("vec3 diffuseLightAccum = vec3(0, 0, 0);");
		//	fsCompiler.mainCode("vec3 specularLightAccum = vec3(0, 0, 0);");
		//	fsCompiler.mainCode("vec3 halfVector;");
		//	for (size_t i = 0; i < dirLights; i++) {
		//		string lightCol = "dirLightColors[" + to_string(i) + "]";
		//		string lightDir = "dirLightDirs[" + to_string(i) + "]";
		//		fsCompiler.mainCode("  diffuseLightAccum += " + lightCol + " * max(dot(norm," + lightDir + "),0);");
		//		fsCompiler.mainCode("halfVector = normalize(" + lightDir + " + fs_in.eye_camera);");
		//		fsCompiler.mainCode("  specularLightAccum += " + lightCol + " * pow(max(dot(norm,halfVector),0), 128);");
		//	}
		//	fsCompiler.mainCode("diffuseTerm = visibility * diffuseLightAccum;");
		//	fsCompiler.mainCode("specularTerm = visibility * specularLightAccum;");
		//}

		//if (useAlpha) fsCompiler.mainCode("color = vec3(ir, ig, ib) * texture2D(imageSampler, fs_in.uv).rgb;");
		//else fsCompiler.mainCode("color = vec4(ir, ig, ib, 1) * texture2D(imageSampler, fs_in.uv);");
		if (useAlpha) fsCompiler.mainCode("color = specularTerm + diffuseTerm * texture2D(imageSampler, fs_in.uv).rgb;");
		else fsCompiler.mainCode("color = vec4(specularTerm, 1.0) + vec4(diffuseTerm, 1.0) * texture2D(imageSampler, fs_in.uv);");

		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void BumpTextureMaterialPass::activate() {
		geometry->activatePositionBuffer(0);
		geometry->activateUVBuffer(1);
		geometry->activateNormalBuffer(2);
		geometry->activateTangentBuffer(3);
		geometry->activateBitangentBuffer(4);
		geometry->activateIndexBuffer();

		glUseProgram(program);
		const glm::mat4 & modelTransform = modelMatrix;
		const glm::mat4 viewTransform = material->getCamera()->getViewMatrix();
		const glm::mat4 mvTransform4x4 = viewTransform * modelTransform;
		const glm::mat3 mvTransform3x3 = glm::mat3(viewTransform * modelTransform);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(material->getVPTransform() * modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvTransform4x4"), 1, false, glm::value_ptr(mvTransform4x4));
		glUniformMatrix3fv(glGetUniformLocation(program, "mvTransform3x3"), 1, false, glm::value_ptr(mvTransform3x3));
		glm::vec3 light_cameraspace = glm::vec3(viewTransform * glm::vec4(light->getPositionVector(), 1));
		glm::vec3 eye_cameraspace = glm::mat3(viewTransform) * material->getCamera()->getEyeVector();
		glUniform3f(glGetUniformLocation(program, "lightPos_camera"), light_cameraspace.x, light_cameraspace.y, light_cameraspace.z);
		glUniform3f(glGetUniformLocation(program, "eyeDir_camera"), -eye_cameraspace.x, -eye_cameraspace.y, -eye_cameraspace.z);
		glUniform1i(glGetUniformLocation(program, "imageSampler"), 0);
		glUniform1i(glGetUniformLocation(program, "normalSampler"), 2);
		glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, imageTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, normalMapTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		auto shadow = material->getShadowMethod();
		if (shadow != nullptr) shadow->activate(program);

		uploadDirectionalLightUniform();
		uploadPointLightUniform();
	}
	void BumpTextureMaterialPass::renderMaterial() {
		if (useAlpha) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		geometry->draw();
		if (useAlpha) glDisable(GL_BLEND);
	}
	void BumpTextureMaterialPass::deactivate() {
		geometry->deactivatePositionBuffer(0);
		geometry->deactivateUVBuffer(1);
		geometry->deactivateNormalBuffer(2);
		geometry->deactivateTangentBuffer(3);
		geometry->deactivateBitangentBuffer(4);
		geometry->deactivateIndexBuffer();
		glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, 0);
		auto shadow = material->getShadowMethod();
		if (shadow != nullptr) shadow->deactivate();
		glUseProgram(0);
	}
	*/

	////////////////////////////////////////////////////////////////////////////////////
	// ShadowTextureMaterial
	ShadowTextureMaterial::ShadowTextureMaterial(GLuint texture) :texture(texture) {
		materialID = MATERIAL_ID::SHADOW_TEXTURE;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// ShadowCubeTextureMaterial
	ShadowCubeTextureMaterial::ShadowCubeTextureMaterial(GLuint texture, unsigned int face, GLfloat zNear, GLfloat zFar)
		: texture(texture), face(face), zNear(zNear), zFar(zFar)
	{
		materialID = MATERIAL_ID::CUBEMAP_SHADOW_TEXTURE;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// WireframeMaterial
	WireframeMaterial::WireframeMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		materialID = MATERIAL_ID::WIREFRAME;
		rgba[0] = r; rgba[1] = g; rgba[2] = b; rgba[3] = a;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// CubeEnvMapMaterial
	CubeEnvMapMaterial::CubeEnvMapMaterial(GLuint cubeTexture) :texture(cubeTexture) {
		materialID = MATERIAL_ID::CUBE_ENV_MAP;
	}

}