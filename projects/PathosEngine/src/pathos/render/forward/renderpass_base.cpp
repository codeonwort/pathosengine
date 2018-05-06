#include "renderpass_base.h"
#include <algorithm>

namespace pathos {

	MeshRenderPass::~MeshRenderPass() {
		if (program) glDeleteProgram(program);
	}

	/*void MeshRenderPass::clearCompilers() {
		vsSource.clear();
		fsSource.clear();
	}
	void MeshRenderPass::createProgram(std::string& vsCode, std::string& fsCode) {
		if (program != 0) glDeleteProgram(program);
		program = pathos::createProgram(vsCode, fsCode);
	}*/

	void MeshRenderPass::uploadDirectionalLightUniform(Scene* scene, unsigned int maxDirectionalLights) {
		if (program == 0) return;
		unsigned int numDirLights = std::min(static_cast<unsigned int>(scene->directionalLights.size()), maxDirectionalLights);
		glUniform1ui(glGetUniformLocation(program, "numDirLights"), numDirLights);
		if (numDirLights > 0) {
			glUniform3fv(glGetUniformLocation(program, "dirLightDirs"), numDirLights, scene->getDirectionalLightDirectionBuffer());
			glUniform3fv(glGetUniformLocation(program, "dirLightColors"), numDirLights, scene->getDirectionalLightColorBuffer());
		}
	}
	void MeshRenderPass::uploadPointLightUniform(Scene* scene, unsigned int maxPointLights) {
		if (program == 0) return;
		unsigned int numPointLights = std::min(static_cast<unsigned int>(scene->pointLights.size()), maxPointLights);
		glUniform1ui(glGetUniformLocation(program, "numPointLights"), numPointLights);
		if (numPointLights) {
			glUniform3fv(glGetUniformLocation(program, "pointLightPos"), numPointLights, scene->getPointLightPositionBuffer());
			glUniform3fv(glGetUniformLocation(program, "pointLightColors"), numPointLights, scene->getPointLightColorBuffer());
		}
	}

}