#include "pathos/render/render.h"
#include "pathos/render/shader.h"
#include "pathos/mesh/shadow.h"
#include <iostream>

namespace pathos {

	void MeshDefaultRenderer::ready() {
		ShadowMethod::clearShadowTextures();
	}

	void MeshDefaultRenderer::render(Scene* scene, Camera* camera) {
		// Currently no optimization. Just render all objects.
		if(scene->skybox != nullptr) render(scene->skybox, camera);
		for (Mesh* mesh : scene->meshes) {
			render(mesh, camera);
		}
	}

	void MeshDefaultRenderer::render(Skybox* sky, Camera* camera) {
		glm::mat4 viewTransform = camera->getViewMatrix();
		sky->activate(viewTransform);
		sky->render();
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
			M->setCamera(camera);
			M->updateProgram();
			renderSub(G, M);
		}

		if (mesh->getDoubleSided()) glEnable(GL_CULL_FACE);
		if (mesh->getRenderInternal()) glFrontFace(GL_CCW);
	}

	void MeshDefaultRenderer::renderSub(MeshGeometry* G, shared_ptr<MeshMaterial> M) {
		M->setGeometry(G);
		// draw depth map
		auto shadow = M->getShadowMethod();
		if (shadow != nullptr) {
			shadow->setTarget(G, M->getModelMatrix());
			shadow->renderDepth();
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

	///////////////////////////////////////////////////////////////
	// NormalRenderer
	NormalRenderer::NormalRenderer(float normLen):normalLength(normLen) {
		vector<ShaderSource*> shaders;
		VertexShaderSource* vs = new VertexShaderSource;
		GeometryShaderSource* gs = new GeometryShaderSource("triangles", "line_strip", 2);
		FragmentShaderSource* fs = new FragmentShaderSource;
		shaders.push_back(vs);
		shaders.push_back(gs);
		shaders.push_back(fs);

		// vs
		vs->setUseNormal(true);
		vs->outVar("vec4", "normalAdded");
		vs->uniform("float", "normalLength");
		vs->mainCode("vs_out.normalAdded = mvpTransform * vec4(position + normal * normalLength, 1);");

		// gs
		gs->inVar("vec3", "normal");
		gs->inVar("vec4", "normalAdded");
		gs->mainCode("gl_Position = gl_in[0].gl_Position;");
		gs->mainCode("EmitVertex();");
		gs->mainCode("gl_Position = gs_in[0].normalAdded;");
		gs->mainCode("EmitVertex();");
		gs->mainCode("EndPrimitive();");

		// fs
		fs->interfaceBlockName("GS_OUT");
		fs->outVar("vec4", "color");
		fs->mainCode("color = vec4(1, 0, 0, 1);");

		cout << endl << vs->getCode() << endl << endl;
		cout << endl << gs->getCode() << endl << endl;
		cout << endl << fs->getCode() << endl << endl;

		program = createProgram(shaders);
	}

	void NormalRenderer::render(Mesh* mesh, Camera* camera) {
		glm::mat4 model = mesh->getTransform().getMatrix();
		glm::mat4 mvp = camera->getViewProjectionMatrix() * model;

		glUseProgram(program);
		glUniform1f(glGetUniformLocation(program, "normalLength"), normalLength);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelTransform"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "mvpTransform"), 1, GL_FALSE, &mvp[0][0]);

		//glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		Geometries geoms = mesh->getGeometries();
		for (auto i = 0; i < geoms.size(); i++){
			geoms[i]->activateVertexBuffer(0);
			geoms[i]->activateNormalBuffer(2);
			geoms[i]->activateIndexBuffer();
			geoms[i]->draw();
			geoms[i]->deactivateVertexBuffer(0);
			geoms[i]->deactivateNormalBuffer(2);
			geoms[i]->deactivateIndexBuffer();
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glEnable(GL_CULL_FACE);

		glUseProgram(0);
	}

}