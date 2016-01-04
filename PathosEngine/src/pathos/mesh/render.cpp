#include <pathos/mesh/render.h>
#include <pathos/mesh/shadow.h>
#include <iostream>

namespace pathos {

	void MeshDefaultRenderer::ready() {
		ShadowMap::clearShadowTextures();
	}

	void MeshDefaultRenderer::render(Mesh* mesh, Camera* camera) {
		glm::mat4 modelTransform = mesh->getTransform().getMatrix();
		glm::mat4 vpTransform = camera->getViewProjectionMatrix();
		glm::vec3 eye = camera->getEyeVector();

		Geometries geoms = mesh->getGeometries();
		Materials materials = mesh->getMaterials();
		size_t len = geoms.size();

		if (mesh->getDoubleSided()) glDisable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CW);

		for (auto i = 0; i < len; i++) {
			auto G = geoms[i];
			auto M = materials[i];
			M->setModelMatrix(modelTransform);
			M->setVPTransform(vpTransform);
			M->setEyeVector(eye);
			M->updateProgram();
			renderSub(G, M);
		}

		if (mesh->getDoubleSided()) glEnable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CCW);
	}

	void MeshDefaultRenderer::renderSub(MeshGeometry* G, shared_ptr<MeshMaterial> M) {
		M->setGeometry(G);
		// draw depth map
		if (M->getShadowMethod() != nullptr) {
			ShadowMap* sm = M->getShadowMethod();
			sm->setGeometry(G);
			sm->activate(M->getModelMatrix());
			sm->renderDepth();
			sm->deactivate();
		}
		/*
		// fill stencil buffer for reflection
		PlaneReflection* reflection = material->getReflectionMethod();
		if (reflection != nullptr) {
			reflection->setGeometry(subGeom);
			reflection->activate(material->getVPTransform() * modelMatrix);
			reflection->renderStencil();
			reflection->deactivate();
		}*/
		// run each pass
		glDepthFunc(GL_LEQUAL);
		for (size_t j = 0; j < M->numPasses(); j++) {
			if (M->isPassEnabled(j) == false) continue;
			M->activatePass(j);
			M->renderPass(j);
			M->deactivatePass(j);
		}
		// draw reflection target
		/*if (reflection != nullptr && (reflection->getTargets()).size() > 0) {
			auto targets = reflection->getTargets();
			glFrontFace(GL_CW);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
			// impossible - target's material passes override the blend setting!
			//glEnable(GL_BLEND);
			//glBlendColor(0, 0, 0, 0.2);
			//glBlendFunc(a, b);
			for (auto i = 0; i < targets.size(); i++) {
				if (reflection->positiveSide(modelMatrix, i)) {
					glm::mat4 reflectionMatrix = reflection->getReflection(modelMatrix, i);
					renderMesh(targets[i], reflectionMatrix, material->getVPTransform(), material->getEyeVector());
				}
			}
			glFrontFace(GL_CCW);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_STENCIL_TEST);
			//glDisable(GL_BLEND);
		}*/
		glDepthFunc(GL_LESS);
	}

}