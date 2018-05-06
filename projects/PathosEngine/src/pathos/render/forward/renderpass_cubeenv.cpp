#include "renderpass_cubeenv.h"
#include "glm/gtc/type_ptr.hpp"

//#define DEBUG_CUBE_ENV_MAPPING

#if defined(_DEBUG) && defined(DEBUG_CUBE_ENV_MAPPING)
#include <iostream>
using namespace std;
#endif

namespace pathos {

	CubeEnvMapPass::CubeEnvMapPass() {
		createProgram();
	}

	void CubeEnvMapPass::createProgram() {
		vsSource.setUseNormal(true);

		fsSource.uniform("vec3", "eye");
		fsSource.uniform("samplerCube", "tex_cubemap");
		fsSource.inVar("vec3", "normal");
		fsSource.outVar("vec4", "outColor");
		fsSource.mainCode("vec3 n = vec3(fs_in.normal.x, -fs_in.normal.y, fs_in.normal.z);");
		//fsSource.mainCode("vec3 r = reflect(-eye, n);");
		//fsSource.mainCode("outColor = texture(tex_cubemap, r);");
		fsSource.mainCode("outColor = texture(tex_cubemap, n);");

		/*vsSource.setUseNormal(true);
		vsSource.uniform("mat4", "mv_matrix");
		vsSource.outVar("vec3", "viewVector");
		vsSource.outVar("vec3", "normal_mv");
		vsSource.mainCode("vec4 pos_vs = modelTransform * vec4(position, 1.0);");
		vsSource.mainCode("vs_out.normal_mv = mat3(modelTransform) * normal;");
		vsSource.mainCode("vec4 pos_vs = modelTransform * vec4(position.x, -position.y, -position.z, 1.0);");
		vsSource.mainCode("vs_out.normal_mv = mat3(modelTransform) * vec3(normal.x, -normal.yz);");
		vsSource.mainCode("vs_out.viewVector = pos_vs.xyz;");

		fsSource.uniform("samplerCube", "tex_cubemap");
		fsSource.uniform("vec3", "cameraPos");
		fsSource.inVar("vec3", "normal");
		fsSource.inVar("vec3", "viewVector");
		fsSource.inVar("vec3", "normal_mv");
		fsSource.outVar("vec4", "outColor");
		fsSource.mainCode("vec3 r = reflect(-fs_in.viewVector, normalize(fs_in.normal_mv));");
		fsSource.mainCode("vec3 r = fs_in.viewVector;");
		fsSource.mainCode("outColor = texture(tex_cubemap, r);");*/

#if defined(_DEBUG) && defined(DEBUG_CUBE_ENV_MAPPING)
		cout << endl << "[renderpass_cubeenv.cpp]" << endl;
		cout << vsSource.getCode() << endl << endl;
		cout << fsSource.getCode() << endl << endl;
#endif

		program = pathos::createProgram(vsSource.getCode(), fsSource.getCode());
		positionLocation = vsSource.getPositionLocation();
		normalLocation = vsSource.getNormalLocation();
		uniform_eye = glGetUniformLocation(program, "eye");
		uniform_sampler = glGetUniformLocation(program, "tex_cubemap");
	}

	void CubeEnvMapPass::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		static_cast<void>(scene);
		CubeEnvMapMaterial* material = static_cast<CubeEnvMapMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activatePositionBuffer(positionLocation);
		geometry->activateNormalBuffer(normalLocation);
		geometry->activateIndexBuffer();

		glUseProgram(program);

		const glm::mat4& modelTransform = modelMatrix;
		//const glm::mat4& mvTransform = camera->getViewMatrix() * modelMatrix;
		const glm::mat4& mvpTransform = camera->getViewProjectionMatrix() * modelTransform;
		//glm::vec3 cameraPos = camera->getPosition();
		//glUniformMatrix4fv(glGetUniformLocation(program, "mv_matrix"), 1, GL_FALSE, glm::value_ptr(mvTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, GL_FALSE, glm::value_ptr(modelTransform));
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, GL_FALSE, glm::value_ptr(mvpTransform));
		//glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

		glm::vec3 eye = camera->getEyeVector();
		glUniform3fv(uniform_eye, 1, &camera->getEyeVector()[0]);

		glUniform1i(uniform_sampler, CubeEnvMapPass::TEXTURE_UNIT);
		glActiveTexture(GL_TEXTURE0 + CubeEnvMapPass::TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_CUBE_MAP, material->getTexture());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		geometry->deactivatePositionBuffer(positionLocation);
		geometry->deactivateNormalBuffer(normalLocation);
		geometry->deactivateIndexBuffer();
		glUseProgram(0);
	}

}