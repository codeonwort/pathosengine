#include <pathos/light/light.h>

namespace pathos {

	// point light
	PointLight::PointLight(const glm::vec3& position, const glm::vec3& color) {
		this->position = position;
		this->color = color;
		for (int i = 0; i < 3; i++){
			posPtr[i] = position[i];
			colorPtr[i] = color[i];
		}
	}

	GLfloat* PointLight::getColor() { return colorPtr; }
	GLfloat* PointLight::getPosition() { return posPtr; }

	// directional light
	DirectionalLight::DirectionalLight(const glm::vec3& direction, const glm::vec3& color) {
		this->direction = glm::normalize(direction);
		this->color = color;
		for (int i = 0; i < 3; i++){
			directionPtr[i] = this->direction[i];
			colorPtr[i] = color[i];
		}
	}

	GLfloat* DirectionalLight::getColor() { return colorPtr; }
	GLfloat* DirectionalLight::getDirection() { return directionPtr; }

}

/* shader code for lighting
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
*/