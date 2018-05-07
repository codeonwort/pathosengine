#include "deferredpass_pack_bumptexture.h"
#include "glm/gtc/type_ptr.hpp"

namespace pathos {

	static constexpr unsigned int DIFFUSE_TEXTURE_UNIT = 0;
	static constexpr unsigned int NORMALMAP_TEXTURE_UNIT = 1;

	MeshDeferredRenderPass_Pack_BumpTexture::MeshDeferredRenderPass_Pack_BumpTexture() {
		createProgram();
	}

	void MeshDeferredRenderPass_Pack_BumpTexture::createProgram() {
		Shader vs(GL_VERTEX_SHADER);
		Shader fs(GL_FRAGMENT_SHADER);
		vs.loadSource("deferred_pack_bumptexture_vs.glsl");
		fs.loadSource("deferred_pack_bumptexture_fs.glsl");

		program = pathos::createProgram(vs, fs);

#define GET_UNIFORM(z) { assert((uniform_##z = glGetUniformLocation(program, #z)) != -1); }
		GET_UNIFORM(mvTransform3x3);
		GET_UNIFORM(mvTransform);
		GET_UNIFORM(mvpTransform);
#undef GET_UNIFORM

		positionLocation = 0;
		uvLocation = 1;
		normalLocation = 2;
		tangentLocation = 3;
		bitangentLocation = 4;
	}

	void MeshDeferredRenderPass_Pack_BumpTexture::render(Scene* scene, Camera* camera, MeshGeometry* geometry, MeshMaterial* material_) {
		BumpTextureMaterial* material = static_cast<BumpTextureMaterial*>(material_);

		//--------------------------------------------------------------------------------------
		// activate
		//--------------------------------------------------------------------------------------
		geometry->activatePositionBuffer(positionLocation);
		geometry->activateUVBuffer(uvLocation);
		geometry->activateNormalBuffer(normalLocation);
		geometry->activateTangentBuffer(tangentLocation);
		geometry->activateBitangentBuffer(bitangentLocation);
		geometry->activateIndexBuffer();

		glUseProgram(program);

		// uniform: transform
		glm::mat4 mvMatrix = camera->getViewMatrix() * modelMatrix;
		glm::mat3 mvMatrix3x3 = glm::mat3(mvMatrix);
		glm::mat4 mvpMatrix = camera->getViewProjectionMatrix() * modelMatrix;
		glUniformMatrix3fv(uniform_mvTransform3x3, 1, false, glm::value_ptr(mvMatrix3x3));
		glUniformMatrix4fv(uniform_mvTransform, 1, false, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(uniform_mvpTransform, 1, false, glm::value_ptr(mvpMatrix));

		// uniform: lighting
		/* no need
		glm::vec3 lightDirection(0.0f, 0.0f, -1.0f);
		if (scene->numDirectionalLights() > 0u) {
			auto dir = scene->directionalLights[0]->getDirection();
			lightDirection = mvMatrix3x3 * glm::vec3(dir[0], dir[1], dir[2]);
		}
		glm::vec3 eyeDirection = mvMatrix3x3 * camera->getEyeVector();
		glUniform3fv(uniform_lightDirection, 1, glm::value_ptr(lightDirection));
		glUniform3fv(uniform_lightDirection, 1, glm::value_ptr(eyeDirection));
		*/

		// uniform: texture
		glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getDiffuseTexture());
		glActiveTexture(GL_TEXTURE0 + NORMALMAP_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, material->getNormalMapTexture());

		//--------------------------------------------------------------------------------------
		// draw call
		//--------------------------------------------------------------------------------------
		geometry->draw();

		//--------------------------------------------------------------------------------------
		// deactivate
		//--------------------------------------------------------------------------------------
		geometry->deactivatePositionBuffer(positionLocation);
		geometry->deactivateUVBuffer(uvLocation);
		geometry->deactivateNormalBuffer(normalLocation);
		geometry->deactivateTangentBuffer(tangentLocation);
		geometry->deactivateBitangentBuffer(bitangentLocation);
		geometry->deactivateIndexBuffer();

		glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0 + NORMALMAP_TEXTURE_UNIT);
		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);
	}

}