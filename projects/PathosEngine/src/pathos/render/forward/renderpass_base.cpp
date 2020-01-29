#include "renderpass_base.h"
#include <algorithm>

namespace pathos {

	MeshRenderPass::MeshRenderPass()
		: program(0)
	{
	}

	MeshRenderPass::~MeshRenderPass() {
		if (program) {
			glDeleteProgram(program);
		}
	}

	void MeshRenderPass::uploadDirectionalLightUniform(Scene* scene, uint32 maxDirectionalLights) {
		if (program == 0) {
			return;
		}
		uint32 numDirLights = std::min(static_cast<uint32>(scene->directionalLights.size()), maxDirectionalLights);
		glUniform1ui(glGetUniformLocation(program, "numDirLights"), numDirLights);
		if (numDirLights > 0) {
			glUniform3fv(glGetUniformLocation(program, "dirLightDirs"), numDirLights, scene->getDirectionalLightDirectionBuffer());
			glUniform3fv(glGetUniformLocation(program, "dirLightColors"), numDirLights, scene->getDirectionalLightColorBuffer());
		}
	}
	void MeshRenderPass::uploadPointLightUniform(Scene* scene, uint32 maxPointLights) {
		CHECK(0);
		
		if (program == 0) {
			return;
		}
		uint32 numPointLights = std::min(static_cast<uint32>(scene->numPointLights()), maxPointLights);
		glUniform1ui(glGetUniformLocation(program, "numPointLights"), numPointLights);
		// #todo-forward-rendering: Fix
		//if (numPointLights) {
		//	glUniform3fv(glGetUniformLocation(program, "pointLightPos"), numPointLights, scene->getPointLightPositionBuffer());
		//	glUniform3fv(glGetUniformLocation(program, "pointLightColors"), numPointLights, scene->getPointLightColorBuffer());
		//}
	}

}
