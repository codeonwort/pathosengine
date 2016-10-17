#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <pathos/mesh/material.h>

namespace pathos {

	// MeshMaterial
	MeshMaterial::MeshMaterial() { }
	MeshMaterial::~MeshMaterial() {
		for (auto it = passes.begin(); it != passes.end(); it++) {
			delete *it;
		}
	}
	void MeshMaterial::setGeometry(MeshGeometry* related) {
		relatedGeometry = related;
		for (auto it = passes.begin(); it != passes.end(); it++) {
			(*it)->setGeometry(related);
		}
	}
	void MeshMaterial::setModelMatrix(const glm::mat4& modelMatrix) {
		this->modelMatrix = modelMatrix;
		for (auto it = passes.begin(); it != passes.end(); it++) {
			(*it)->setModelMatrix(modelMatrix);
		}
	}
	glm::mat4& MeshMaterial::getModelMatrix() { return modelMatrix; }
	glm::vec3& MeshMaterial::getEyeVector() { return eyeVector; }
	glm::mat4& MeshMaterial::getVPTransform() { return vpTransform; }
	void MeshMaterial::setEyeVector(glm::vec3& eye) { eyeVector = glm::normalize(eye); }
	void MeshMaterial::setVPTransform(glm::mat4& vp) { vpTransform = vp; }

	void MeshMaterial::updateProgram() {
		if (!programDirty) return;
		for (auto it = passes.begin(); it != passes.end(); it++) {
			(*it)->clearCompilers();
			(*it)->setMaterial(this);
			(*it)->updateProgram(this);
		}
		updateDirectionalLightBuffer();
		updatePointLightBuffer();
		programDirty = false;
	}
	void MeshMaterial::updateDirectionalLightBuffer() {
		size_t numLights = directionalLights.size();
		dirLightDirections.clear();
		dirLightColors.clear();
		for (size_t i = 0; i < numLights; i++){
			dirLightDirections.push_back(directionalLights[i]->getDirection()[0]);
			dirLightDirections.push_back(directionalLights[i]->getDirection()[1]);
			dirLightDirections.push_back(directionalLights[i]->getDirection()[2]);
			dirLightColors.push_back(directionalLights[i]->getColor()[0]);
			dirLightColors.push_back(directionalLights[i]->getColor()[1]);
			dirLightColors.push_back(directionalLights[i]->getColor()[2]);
		}
	}
	void MeshMaterial::updatePointLightBuffer() {
		size_t numLights = pointLights.size();
		pointLightPositions.clear();
		pointLightColors.clear();
		for (size_t i = 0; i < numLights; i++){
			pointLightPositions.push_back(pointLights[i]->getPosition()[0]);
			pointLightPositions.push_back(pointLights[i]->getPosition()[1]);
			pointLightPositions.push_back(pointLights[i]->getPosition()[2]);
			pointLightColors.push_back(pointLights[i]->getColor()[0]);
			pointLightColors.push_back(pointLights[i]->getColor()[1]);
			pointLightColors.push_back(pointLights[i]->getColor()[2]);
		}
	}

	void MeshMaterial::addPass(MeshMaterialPass* pass) {
		passes.push_back(pass);
		pass->setMaterial(this);
		enabled.push_back(true);
	}
	size_t MeshMaterial::numPasses() { return passes.size(); }

	bool MeshMaterial::isPassEnabled(int index) { return enabled[index]; }
	void MeshMaterial::enablePass(int index) { enabled[index] = true; }
	void MeshMaterial::disablePass(int index) { enabled[index] = false; }

	void MeshMaterial::activatePass(int i) { passes[i]->activate(); }
	void MeshMaterial::renderPass(int i) { passes[i]->renderMaterial(); }
	void MeshMaterial::deactivatePass(int i) { passes[i]->deactivate(); }

	void MeshMaterial::addLight(DirectionalLight* light) {
		if (std::find(directionalLights.begin(), directionalLights.end(), light) != directionalLights.end()) {
			// this light is already applied. do nothing
			return;
		}
		directionalLights.push_back(light);
		programDirty = true;
	}
	void MeshMaterial::addLight(PointLight* light) {
		if (std::find(pointLights.begin(), pointLights.end(), light) != pointLights.end()) {
			// this light is already applied. do nothing
			return;
		}
		pointLights.push_back(light);
		programDirty = true;
	}
	const std::vector<DirectionalLight*>& MeshMaterial::getDirectionalLights() { return directionalLights; }
	const std::vector<PointLight*>& MeshMaterial::getPointLights() { return pointLights; }
	
	/*PlaneReflection* MeshMaterial::getReflectionMethod() { return reflectionMethod; }
	void MeshMaterial::setReflectionMethod(PlaneReflection* pr) {
		reflectionMethod = pr;
		pr->setReflector(owner);
		programDirty = true;
	}*/

	// MeshMaterialPass
	MeshMaterialPass::MeshMaterialPass() {
		program = 0;
	}
	MeshMaterialPass::~MeshMaterialPass() {
		if (program) glDeleteProgram(program);
	}
	void MeshMaterialPass::setMaterial(MeshMaterial* M) { material = M; }
	void MeshMaterialPass::setGeometry(MeshGeometry* geom) { geometry = geom; }
	void MeshMaterialPass::setModelMatrix(const glm::mat4& modelMatrix) { this->modelMatrix = modelMatrix; }

	void MeshMaterialPass::clearCompilers() {
		vsCompiler.clear();
		fsCompiler.clear();
	}
	void MeshMaterialPass::createProgram(std::string& vsCode, std::string& fsCode) {
		if (program != 0) glDeleteProgram(program);
		program = pathos::createProgram(vsCode, fsCode);
	}
	void MeshMaterialPass::uploadDirectionalLightUniform() {
		size_t numDirLights = material->getDirectionalLights().size();
		if (numDirLights > 0) {
			glUniform3fv(glGetUniformLocation(program, "dirLightDirs"), numDirLights, &material->getDirectionalLightDirectionBuffer()[0]);
			glUniform3fv(glGetUniformLocation(program, "dirLightColors"), numDirLights, &material->getDirectionalLightColorBuffer()[0]);
		}
	}
	void MeshMaterialPass::uploadPointLightUniform() {
		size_t numPointLights = material->getPointLights().size();
		if (numPointLights > 0){
			glUniform3fv(glGetUniformLocation(program, "pointLightPos"), numPointLights, &material->getPointLightPositionBuffer()[0]);
			glUniform3fv(glGetUniformLocation(program, "pointLightColors"), numPointLights, &material->getPointLightColorBuffer()[0]);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// ColorMaterial
	ColorMaterial::ColorMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { this->init(r, g, b, a); }
	ColorMaterial::ColorMaterial(const glm::vec3& rgb) { this->init(rgb[0], rgb[1], rgb[2], 1.0f); }
	ColorMaterial::ColorMaterial(const glm::vec4& rgba) { this->init(rgba[0], rgba[1], rgba[2], rgba[3]); }

	void ColorMaterial::setAmbientColor(GLfloat r, GLfloat g, GLfloat b) { pass->setAmbient(r, g, b); }
	void ColorMaterial::setDiffuseColor(GLfloat r, GLfloat g, GLfloat b) { pass->setDiffuse(r, g, b); }
	void ColorMaterial::setSpecularColor(GLfloat r, GLfloat g, GLfloat b) { pass->setSpecular(r, g, b); }
	void ColorMaterial::setAlpha(GLfloat a) { pass->setAlpha(a); }
	void ColorMaterial::setBlendFactor(GLuint srcFactor, GLuint dstFactor) { pass->setBlendFactor(srcFactor, dstFactor); }

	void ColorMaterial::init(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		pass = new ColorMaterialPass(r, g, b, a);
		addPass(pass);
	}

	// ColorMaterialPass
	ColorMaterialPass::ColorMaterialPass(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		diffuse[0] = r; diffuse[1] = g; diffuse[2] = b;
		ambient[0] = ambient[1] = ambient[2] = 0;
		specular[0] = specular[1] = specular[2] = 0;
		alpha = a;
	}
	void ColorMaterialPass::setAmbient(GLfloat r, GLfloat g, GLfloat b) { ambient[0] = r; ambient[1] = g; ambient[2] = b; }
	void ColorMaterialPass::setDiffuse(GLfloat r, GLfloat g, GLfloat b) { diffuse[0] = r; diffuse[1] = g; diffuse[2] = b; }
	void ColorMaterialPass::setSpecular(GLfloat r, GLfloat g, GLfloat b) { specular[0] = r; specular[1] = g; specular[2] = b; }
	void ColorMaterialPass::setAlpha(GLfloat a) { alpha = a; }
	void ColorMaterialPass::setBlendFactor(GLuint srcFactor, GLuint dstFactor) { blendSrcFactor = srcFactor; blendDstFactor = dstFactor; }
	void ColorMaterialPass::updateProgram(MeshMaterial* M) {
		// light
		size_t dirLights = M->getDirectionalLights().size();
		if (dirLights > 0) {
			vsCompiler.setUseNormal(true);
			fsCompiler.directionalLights(dirLights);
			fsCompiler.inVar("vec3", "normal");
		}
		size_t pointLights = M->getPointLights().size();
		if (pointLights > 0){
			vsCompiler.setUseNormal(true);
			vsCompiler.setTransferPosition(true);
			fsCompiler.inVar("vec3", "position");
			fsCompiler.pointLights(pointLights);
			fsCompiler.inVar("vec3", "normal");
		}

		// shadow
		fsCompiler.mainCode("float visibility = 1.0;");
		if (M->getShadowMethod() != nullptr) {
			M->getShadowMethod()->addShaderCode(vsCompiler, fsCompiler);
		}

		fsCompiler.uniform("vec3", "ambientColor");
		fsCompiler.uniform("vec3", "diffuseColor");
		fsCompiler.uniform("vec3", "specularColor");
			fsCompiler.uniform("vec3", "eye");
		fsCompiler.uniform("float", "materialAlpha");
		fsCompiler.outVar("vec4", "outColor");

		// directional lighting
		fsCompiler.mainCode("vec3 diffuseTerm = vec3(0, 0, 0);");
		fsCompiler.mainCode("vec3 specularTerm = vec3(0, 0, 0);");
		if (dirLights > 0) {
			fsCompiler.mainCode("vec3 norm = normalize(fs_in.normal);");
			fsCompiler.mainCode("vec3 diffuseLightAccum = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 specularLightAccum = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 halfVector;");
			for (size_t i = 0; i < dirLights; i++) {
				string lightCol = "dirLightColors[" + to_string(i) + "]";
				string lightDir = "dirLightDirs[" + to_string(i) + "]";
				fsCompiler.mainCode("  diffuseLightAccum += " + lightCol + " * max(dot(norm," + lightDir + "),0);");
				fsCompiler.mainCode("halfVector = normalize(" + lightDir + " + eye);");
				fsCompiler.mainCode("  specularLightAccum += " + lightCol + " * pow(max(dot(norm,halfVector),0), 128);");
			}
			fsCompiler.mainCode("diffuseTerm = visibility*diffuseColor*diffuseLightAccum;");
			fsCompiler.mainCode("specularTerm = visibility*specularColor*specularLightAccum;");
		}
		// point lighting
		if (pointLights > 0) {
			fsCompiler.mainCode("vec3 norm2 = normalize(fs_in.normal);"); // normal in world space
			fsCompiler.mainCode("vec3 diffuseLightAccum2 = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 specularLightAccum2 = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 halfVector2;");
			for (size_t i = 0; i < pointLights; i++) {
				string lightCol = "pointLightColors[" + to_string(i) + "]";
				string lightDir = "normalize(pointLightPos[" + to_string(i) + "] - fs_in.position)";
				fsCompiler.mainCode("  diffuseLightAccum2 += " + lightCol + " * max(dot(norm2," + lightDir + "),0);");
				fsCompiler.mainCode("halfVector2 = normalize(" + lightDir + " + eye);");
				fsCompiler.mainCode("  specularLightAccum2 += " + lightCol + " * pow(max(dot(norm2,halfVector2),0), 128);");
			}
			fsCompiler.mainCode("diffuseTerm += visibility*diffuseColor*diffuseLightAccum2;");
			fsCompiler.mainCode("specularTerm += visibility*specularColor*specularLightAccum2;");
		}
		// final shading
		fsCompiler.mainCode("outColor.rgb = ambientColor + diffuseTerm + specularTerm;");
		fsCompiler.mainCode("outColor.a = materialAlpha;");

		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void ColorMaterialPass::activate() {
		geometry->activateVertexBuffer(0);
		geometry->activateIndexBuffer();
		if (material->getDirectionalLights().size() > 0 || material->getPointLights().size() > 0) {
			geometry->activateNormalBuffer(2);
		}

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(material->getVPTransform() * modelMatrix));
		glUniform3fv(glGetUniformLocation(program, "ambientColor"), 1, ambient);
		glUniform3fv(glGetUniformLocation(program, "diffuseColor"), 1, diffuse);
		glUniform3fv(glGetUniformLocation(program, "specularColor"), 1, specular);
		glm::vec3& eye = material->getEyeVector();
		glUniform3f(glGetUniformLocation(program, "eye"), -eye.x, -eye.y, -eye.z);
		glUniform1f(glGetUniformLocation(program, "materialAlpha"), alpha);

		auto shadow = material->getShadowMethod();
		if (shadow != nullptr) shadow->activate(program);

		uploadDirectionalLightUniform();
		uploadPointLightUniform();

		glEnable(GL_BLEND);
		glBlendFunc(blendSrcFactor, blendDstFactor);
	}
	void ColorMaterialPass::renderMaterial() {
		geometry->draw();
	}
	void ColorMaterialPass::deactivate() {
		geometry->deactivateVertexBuffer(0);
		geometry->deactivateIndexBuffer();
		if (material->getDirectionalLights().size() > 0) {
			geometry->deactivateNormalBuffer(2);
		}
		glDisable(GL_BLEND);

		auto shadow = material->getShadowMethod();
		if (shadow != nullptr) shadow->deactivate();

		glUseProgram(0);
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// TextureMaterial
	TextureMaterial::TextureMaterial(GLuint texID, bool useAlpha, string channelMapping) {
		addPass(pass0 = new TextureMaterialPass(texID, useAlpha, channelMapping));
	}
	void TextureMaterial::setLighting(bool value) {
		pass0->setLighting(value);
	}

	// TextureMaterialPass
	TextureMaterialPass::TextureMaterialPass(GLuint texID, bool useAlpha, string channelMapping) : textureID(texID), channelMapping(channelMapping), useAlpha(useAlpha) {
		useLighting = true;
	}
	void TextureMaterialPass::setLighting(bool value) {
		useLighting = value;
	}
	void TextureMaterialPass::updateProgram(MeshMaterial* M) {
		vsCompiler.setUseUV(true);
		vsCompiler.setUVLocation(1);
		fsCompiler.textureSampler("texSampler");
		fsCompiler.inVar("vec2", "uv");
		useAlpha ? fsCompiler.outVar("vec4", "color") : fsCompiler.outVar("vec3", "color");

		// light
		size_t dirLights = M->getDirectionalLights().size();
		size_t pointLights = M->getPointLights().size();
		if (useLighting){
			fsCompiler.uniform("vec3", "eye");
			if (dirLights > 0) {
				vsCompiler.setUseNormal(true);
				fsCompiler.directionalLights(dirLights);
				fsCompiler.inVar("vec3", "normal");
			}
			if (pointLights > 0){
				vsCompiler.setUseNormal(true);
				vsCompiler.setTransferPosition(true);
				fsCompiler.inVar("vec3", "position");
				fsCompiler.pointLights(pointLights);
				fsCompiler.inVar("vec3", "normal");
			}

		}

		// shadow
		fsCompiler.mainCode("float visibility = 1.0;");
		if (M->getShadowMethod() != nullptr) {
			M->getShadowMethod()->addShaderCode(vsCompiler, fsCompiler);
		}

		if (useLighting){
			fsCompiler.mainCode("vec3 diffuseLightAccum = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 specularLightAccum = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 diffuseLightAccum2 = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 specularLightAccum2 = vec3(0, 0, 0);");
			// directional lighting
			if (dirLights > 0) {
				fsCompiler.mainCode("vec3 norm = normalize(fs_in.normal);");
				fsCompiler.mainCode("vec3 halfVector;");
				for (size_t i = 0; i < dirLights; i++) {
					string lightCol = "dirLightColors[" + to_string(i) + "]";
					string lightDir = "dirLightDirs[" + to_string(i) + "]";
					fsCompiler.mainCode("  diffuseLightAccum += " + lightCol + " * max(dot(norm," + lightDir + "),0);");
					fsCompiler.mainCode("halfVector = normalize(" + lightDir + " + eye);");
					fsCompiler.mainCode("  specularLightAccum += " + lightCol + " * pow(max(dot(norm,halfVector),0), 128);");
				}
			}
			// point lighting
			if (pointLights > 0) {
				fsCompiler.mainCode("vec3 norm2 = normalize(fs_in.normal);"); // normal in world space
				fsCompiler.mainCode("vec3 halfVector2;");
				for (size_t i = 0; i < pointLights; i++) {
					string lightCol = "pointLightColors[" + to_string(i) + "]";
					string lightDir = "normalize(pointLightPos[" + to_string(i) + "] - fs_in.position)";
					fsCompiler.mainCode("  diffuseLightAccum2 += " + lightCol + " * max(dot(norm2," + lightDir + "),0);");
					fsCompiler.mainCode("halfVector2 = normalize(" + lightDir + " + eye);");
					fsCompiler.mainCode("  specularLightAccum2 += " + lightCol + " * pow(max(dot(norm2,halfVector2),0), 128);");
				}
			}
		}

		string colorOut = "color = visibility * texture2D(texSampler, fs_in.uv).";
		colorOut += channelMapping;
		colorOut += useAlpha ? "a;" : ";";
		fsCompiler.mainCode(colorOut);
		if (useLighting){
			colorOut = "color.rgb = color.rgb * (diffuseLightAccum + diffuseLightAccum2 + specularLightAccum + specularLightAccum2);";
			fsCompiler.mainCode(colorOut);
		}

		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void TextureMaterialPass::activate() {
		geometry->activateVertexBuffer(0);
		geometry->activateUVBuffer(1);
		geometry->activateIndexBuffer();

		// lighting related
		if (useLighting){
			if (material->getDirectionalLights().size() > 0 || material->getPointLights().size() > 0) {
				geometry->activateNormalBuffer(2);
			}
			glm::vec3& eye = material->getEyeVector();
			glUniform3f(glGetUniformLocation(program, "eye"), -eye.x, -eye.y, -eye.z);
		}
		
		glUseProgram(program);
		const glm::mat4 & modelTransform = modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(material->getVPTransform() * modelTransform));
		glUniform1i(glGetUniformLocation(program, "texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		auto shadow = material->getShadowMethod();
		if (shadow != nullptr) shadow->activate(program);

		uploadDirectionalLightUniform();
		uploadPointLightUniform();
	}
	void TextureMaterialPass::renderMaterial() {
		if (useAlpha) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		geometry->draw();
		if (useAlpha) glDisable(GL_BLEND);
	}
	void TextureMaterialPass::deactivate() {
		geometry->deactivateVertexBuffer(0);
		geometry->deactivateUVBuffer(1);
		geometry->deactivateIndexBuffer();
		glBindTexture(GL_TEXTURE_2D, 0);
		auto shadow = material->getShadowMethod();
		if (shadow != nullptr) shadow->deactivate();
		if (material->getDirectionalLights().size() > 0 || material->getPointLights().size() > 0) {
			geometry->deactivateNormalBuffer(2);
		}
		glUseProgram(0);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// BumpTextureMaterial
	BumpTextureMaterial::BumpTextureMaterial(GLuint imageTexture, GLuint normalMapTexture, PointLight* light, bool useAlpha) {
		addPass(new BumpTextureMaterialPass(imageTexture, normalMapTexture, light, useAlpha));
	}

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

		/*if (dirLights > 0) {
			fsCompiler.mainCode("vec3 diffuseLightAccum = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 specularLightAccum = vec3(0, 0, 0);");
			fsCompiler.mainCode("vec3 halfVector;");
			for (size_t i = 0; i < dirLights; i++) {
				string lightCol = "dirLightColors[" + to_string(i) + "]";
				string lightDir = "dirLightDirs[" + to_string(i) + "]";
				fsCompiler.mainCode("  diffuseLightAccum += " + lightCol + " * max(dot(norm," + lightDir + "),0);");
				fsCompiler.mainCode("halfVector = normalize(" + lightDir + " + fs_in.eye_camera);");
				fsCompiler.mainCode("  specularLightAccum += " + lightCol + " * pow(max(dot(norm,halfVector),0), 128);");
			}
			fsCompiler.mainCode("diffuseTerm = visibility * diffuseLightAccum;");
			fsCompiler.mainCode("specularTerm = visibility * specularLightAccum;");
		}*/

		if (useAlpha) fsCompiler.mainCode("color = specularTerm + diffuseTerm * texture2D(imageSampler, fs_in.uv).rgb;");
		else fsCompiler.mainCode("color = vec4(specularTerm, 1.0) + vec4(diffuseTerm, 1.0) * texture2D(imageSampler, fs_in.uv);");

		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void BumpTextureMaterialPass::activate() {
		geometry->activateVertexBuffer(0);
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
		geometry->deactivateVertexBuffer(0);
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

	////////////////////////////////////////////////////////////////////////////////////
	// ShadowTextureMaterial
	ShadowTextureMaterial::ShadowTextureMaterial(GLuint texID) {
		addPass(new ShadowTextureMaterialPass(texID));
	}

	// ShadowTextureMaterialPass
	ShadowTextureMaterialPass::ShadowTextureMaterialPass(GLuint texID) : debugTexture(texID) {}
	void ShadowTextureMaterialPass::updateProgram(MeshMaterial* M) {
		vsCompiler.setUseUV(true);
		vsCompiler.setUVLocation(1);
		fsCompiler.textureSampler("texSampler");
		fsCompiler.inVar("vec2", "uv");
		fsCompiler.outVar("vec4", "color");
		fsCompiler.mainCode("float depth = texture2D(texSampler, fs_in.uv).r;");
		fsCompiler.mainCode("depth = clamp(depth, 0, 1);");
		fsCompiler.mainCode("color = vec4(depth, depth, depth, 1.0);");
		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void ShadowTextureMaterialPass::activate() {
		geometry->activateVertexBuffer(0);
		geometry->activateUVBuffer(1);
		geometry->activateIndexBuffer();

		glUseProgram(program);
		const glm::mat4 & modelTransform = modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(material->getVPTransform() * modelTransform));
		glUniform1i(glGetUniformLocation(program, "texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, debugTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	void ShadowTextureMaterialPass::renderMaterial() {
		geometry->draw();
	}
	void ShadowTextureMaterialPass::deactivate() {
		geometry->deactivateVertexBuffer(0);
		geometry->deactivateUVBuffer(1);
		geometry->deactivateIndexBuffer();
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// ShadowCubeTextureMaterial
	ShadowCubeTextureMaterial::ShadowCubeTextureMaterial(GLuint texID, unsigned int face) {
		addPass(new ShadowCubeTextureMaterialPass(texID, face));
	}

	// ShadowCubeTextureMaterialPass
	ShadowCubeTextureMaterialPass::ShadowCubeTextureMaterialPass(GLuint texID, unsigned int face) : debugTexture(texID), face(face) {}
	void ShadowCubeTextureMaterialPass::updateProgram(MeshMaterial* M) {
		vsCompiler.setUseUV(true);
		vsCompiler.setUVLocation(1);
		fsCompiler.textureSamplerCube("texSampler");
		fsCompiler.inVar("vec2", "uv");
		fsCompiler.outVar("vec4", "color");
		fsCompiler.mainCode("vec2 uv = fs_in.uv * 2 - vec2(1.0);");
		if (face == 0) fsCompiler.mainCode("float depth = texture(texSampler, vec3(1, uv.y, uv.x)).r;");
		if (face == 1) fsCompiler.mainCode("float depth = texture(texSampler, vec3(-1, uv.y, uv.x)).r;");
		if (face == 2) fsCompiler.mainCode("float depth = texture(texSampler, vec3(uv.y, 1, uv.x)).r;");
		if (face == 3) fsCompiler.mainCode("float depth = texture(texSampler, vec3(uv.y, -1, uv.x)).r;");
		if (face == 4) fsCompiler.mainCode("float depth = texture(texSampler, vec3(uv.x, uv.y, 1)).r;");
		if (face == 5) fsCompiler.mainCode("float depth = texture(texSampler, vec3(uv.x, uv.y, -1)).r;");
		fsCompiler.mainCode("depth = clamp(depth, 0, 1);");
		fsCompiler.mainCode("color = vec4(depth, depth, depth, 1.0);");
		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void ShadowCubeTextureMaterialPass::activate() {
		geometry->activateVertexBuffer(0);
		geometry->activateUVBuffer(1);
		geometry->activateIndexBuffer();

		glUseProgram(program);
		const glm::mat4 & modelTransform = modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, false, glm::value_ptr(modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(material->getVPTransform() * modelTransform));
		glUniform1i(glGetUniformLocation(program, "texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, debugTexture);
	}
	void ShadowCubeTextureMaterialPass::renderMaterial() {
		geometry->draw();
	}
	void ShadowCubeTextureMaterialPass::deactivate() {
		geometry->deactivateVertexBuffer(0);
		geometry->deactivateUVBuffer(1);
		geometry->deactivateIndexBuffer();
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// WireframeMaterial
	WireframeMaterial::WireframeMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		red = r; green = g; blue = b; alpha = a;
		addPass(new WireframeMaterialPass(r, g, b, a));
	}
	// WireframeMaterialPass
	WireframeMaterialPass::WireframeMaterialPass(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
		color[0] = r; color[1] = g; color[2] = b; color[3] = a;
	}
	void WireframeMaterialPass::updateProgram(MeshMaterial*) {
		fsCompiler.uniform("vec4", "color");
		fsCompiler.outVar("vec4", "outColor");
		fsCompiler.mainCode("outColor = color;");
		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void WireframeMaterialPass::activate() {
		geometry->activateVertexBuffer(0);
		geometry->activateIndexBuffer();

		glUseProgram(program);
		const glm::mat4 & modelTransform = modelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, false, glm::value_ptr(material->getVPTransform() * modelTransform));
		glUniform4fv(glGetUniformLocation(program, "color"), 1, color);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	void WireframeMaterialPass::renderMaterial() {
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		geometry->draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}
	void WireframeMaterialPass::deactivate() {
		geometry->deactivateVertexBuffer(0);
		geometry->deactivateIndexBuffer();
		glDisable(GL_BLEND);
		glUseProgram(0);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// CubeEnvMapMaterial
	CubeEnvMapMaterial::CubeEnvMapMaterial(GLuint cubeTexture){
		addPass(new CubeEnvMapMaterialPass(cubeTexture));
	}

	CubeEnvMapMaterialPass::CubeEnvMapMaterialPass(GLuint cubeTexture) : texture(cubeTexture) {}
	void CubeEnvMapMaterialPass::updateProgram(MeshMaterial*) {
		vsCompiler.setUseNormal(true);
		vsCompiler.uniform("mat4", "mv_matrix");
		vsCompiler.outVar("vec3", "viewVector");
		vsCompiler.outVar("vec3", "normal_mv");
		//vsCompiler.mainCode("vec4 pos_vs = modelTransform * vec4(position, 1.0);");
		//vsCompiler.mainCode("vs_out.normal_mv = mat3(modelTransform) * normal;");
		vsCompiler.mainCode("vec4 pos_vs = modelTransform * vec4(position.x, -position.y, -position.z, 1.0);");
		vsCompiler.mainCode("vs_out.normal_mv = mat3(modelTransform) * vec3(normal.x, -normal.yz);");
		vsCompiler.mainCode("vs_out.viewVector = pos_vs.xyz;");

		fsCompiler.uniform("samplerCube", "tex_cubemap");
		fsCompiler.uniform("vec3", "cameraPos");
		fsCompiler.inVar("vec3", "normal");
		fsCompiler.inVar("vec3", "viewVector");
		fsCompiler.inVar("vec3", "normal_mv");
		fsCompiler.outVar("vec4", "color");
		fsCompiler.mainCode("vec3 r = reflect(-fs_in.viewVector, normalize(fs_in.normal_mv));");
		//fsCompiler.mainCode("vec3 r = fs_in.viewVector;");
		fsCompiler.mainCode("color = texture(tex_cubemap, r);");

		cout << vsCompiler.getCode() << endl;
		cout << fsCompiler.getCode() << endl;

		createProgram(vsCompiler.getCode(), fsCompiler.getCode());
	}
	void CubeEnvMapMaterialPass::activate() {
		geometry->activateVertexBuffer(0);
		geometry->activateNormalBuffer(2);
		geometry->activateIndexBuffer();

		glUseProgram(program);
		const glm::mat4& modelTransform = modelMatrix;
		const glm::mat4& mvTransform = material->getCamera()->getViewMatrix() * modelMatrix;
		glm::vec3 cameraPos = material->getCamera()->getPosition();
		glUniformMatrix4fv(glGetUniformLocation(program, "mv_matrix"), 1, GL_FALSE, glm::value_ptr(mvTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, GL_FALSE, glm::value_ptr(modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, GL_FALSE, glm::value_ptr(material->getVPTransform() * modelTransform));
		glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

		glUniform1i(glGetUniformLocation(program, "tex_cubemap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	}
	void CubeEnvMapMaterialPass::renderMaterial() {
		geometry->draw();
	}
	void CubeEnvMapMaterialPass::deactivate() {
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		geometry->deactivateVertexBuffer(0);
		geometry->deactivateNormalBuffer(2);
		geometry->deactivateIndexBuffer();
		glUseProgram(0);
	}

}