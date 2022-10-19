#include "sky_ansel.h"
#include "render_device.h"
#include "pathos/engine_policy.h"
#include "pathos/render/scene_proxy.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/shader/shader_program.h"
#include "pathos/util/gl_debug_group.h"

#include <string>

namespace pathos {
	
	class AnselSkyVS : public ShaderStage {
	public:
		AnselSkyVS() : ShaderStage(GL_VERTEX_SHADER, "AnselSkyVS") {
			addDefine("VERTEX_SHADER", 1);
			if (pathos::getReverseZPolicy() == EReverseZPolicy::Reverse) {
				addDefine("REVERSE_Z", 1);
			}
			setFilepath("sky_ansel.glsl");
		}
	};

	class AnselSkyFS : public ShaderStage {
	public:
		AnselSkyFS() : ShaderStage(GL_FRAGMENT_SHADER, "AnselSkyVS") {
			addDefine("FRAGMENT_SHADER", 1);
			setFilepath("sky_ansel.glsl");
		}
	};

	DEFINE_SHADER_PROGRAM2(Program_AnselSky, AnselSkyVS, AnselSkyFS);

}

namespace pathos {

	void AnselSkyPass::initializeResources(RenderCommandList& cmdList) {
		gRenderDevice->createFramebuffers(1, &fbo);
		cmdList.objectLabel(GL_FRAMEBUFFER, fbo, -1, "FBO_AnselSky");
		cmdList.namedFramebufferDrawBuffer(fbo, GL_COLOR_ATTACHMENT0);

		uniform_transform = 0;
	}

	void AnselSkyPass::releaseResources(RenderCommandList& cmdList) {
		gRenderDevice->deleteFramebuffers(1, &fbo);
	}

	void AnselSkyPass::render(RenderCommandList& cmdList, SceneProxy* scene) {
		SCOPED_DRAW_EVENT(AnselSkyActor);

		const Camera& camera = scene->camera;
		AnselSkyProxy* anselSky = scene->anselSky;

		SceneRenderTargets& sceneContext = *cmdList.sceneRenderTargets;

		const matrix4 view = matrix4(matrix3(camera.getViewMatrix())); // view transform without transition
		const matrix4& proj = camera.getProjectionMatrix();
		const matrix4 transform = proj * view;

		// Write to only far plane.
		cmdList.depthFunc(GL_EQUAL);
		cmdList.enable(GL_DEPTH_TEST);
		cmdList.depthMask(GL_FALSE);

		ShaderProgram& program = FIND_SHADER_PROGRAM(Program_AnselSky);
		cmdList.useProgram(program.getGLName());

		cmdList.bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		cmdList.namedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, sceneContext.sceneColor, 0);
		cmdList.namedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, sceneContext.sceneDepth, 0);

		cmdList.uniformMatrix4fv(uniform_transform, 1, GL_FALSE, &transform[0][0]);
		cmdList.bindTextureUnit(0, anselSky->textureID);

		cmdList.viewport(0, 0, sceneContext.sceneWidth, sceneContext.sceneHeight);

		anselSky->sphere->activate_position(cmdList);
		anselSky->sphere->activateIndexBuffer(cmdList);
		anselSky->sphere->drawPrimitive(cmdList);
	}

}

namespace pathos {

	IcosahedronGeometry::IcosahedronGeometry(uint32_t subdivisionStep /*= 0*/) {
		buildGeometry();
		while (subdivisionStep --> 0) {
			subdivide();
		}
		uploadToGPU();
	}

	void IcosahedronGeometry::buildGeometry() {
		const float X = 0.525731112119133606f;
		const float Z = 0.850650808352039932f;
		const float pi_inv = glm::one_over_pi<float>();

		numVertices = INITIAL_NUM_VERTICES;
		numTriangles = INITIAL_NUM_TRIANLGES;

		GLfloat positionDataSource[INITIAL_NUM_VERTICES][3] = {
			{ -X, 0.0, Z },{ X, 0.0, Z },{ -X, 0.0, -Z },{ X, 0.0, -Z },
			{ 0.0, Z, X },{ 0.0, Z, -X },{ 0.0, -Z, X },{ 0.0, -Z, -X },
			{ Z, X, 0.0 },{ -Z, X, 0.0 },{ Z, -X, 0.0 },{ -Z, -X, 0.0 }
		};
		// cw winding
		GLuint indexDataSource[INITIAL_NUM_TRIANLGES][3] = {
			{ 0,4,1 },{ 0,9,4 },{ 9,5,4 },{ 4,5,8 },{ 4,8,1 },
			{ 8,10,1 },{ 8,3,10 },{ 5,3,8 },{ 5,2,3 },{ 2,7,3 },
			{ 7,10,3 },{ 7,6,10 },{ 7,11,6 },{ 11,0,6 },{ 0,1,6 },
			{ 6,1,10 },{ 9,0,11 },{ 9,11,2 },{ 9,2,5 },{ 7,2,11 }
		};

		positionData = new GLfloat[numVertices * 3];
		indexData = new GLuint[numTriangles * 3];

		memcpy(positionData, positionDataSource, sizeof(GLfloat) * numVertices * 3);
		memcpy(indexData, indexDataSource, sizeof(GLuint) * numTriangles * 3);
	}

	void IcosahedronGeometry::subdivide() {
		unsigned int newNumVertices = numVertices + 3 * numTriangles; // three vertices are created per one triangle
		unsigned int newNumTriangles = numTriangles * 4; // each triangle is divided into four triangles

		GLfloat* newPositions = new GLfloat[newNumVertices * 3];
		GLuint* newIndices = new GLuint[newNumTriangles * 3];

		memcpy(newPositions, positionData, sizeof(GLfloat) * numVertices * 3);

		// build
		unsigned int index_end = numTriangles * 3;
		unsigned int p = numVertices; // position append location
		unsigned int q = 0; // index append location
		for (unsigned int i = 0; i < index_end; i += 3) {
			unsigned int i0 = indexData[i];
			unsigned int i1 = indexData[i + 1];
			unsigned int i2 = indexData[i + 2];

			unsigned int i01, i12, i20;
			i01 = p;
			i12 = p + 1;
			i20 = p + 2;

			glm::vec3 v0(positionData[i0 * 3], positionData[i0 * 3 + 1], positionData[i0 * 3 + 2]);
			glm::vec3 v1(positionData[i1 * 3], positionData[i1 * 3 + 1], positionData[i1 * 3 + 2]);
			glm::vec3 v2(positionData[i2 * 3], positionData[i2 * 3 + 1], positionData[i2 * 3 + 2]);

			glm::vec3 v01 = glm::normalize(v0 + v1);
			glm::vec3 v12 = glm::normalize(v1 + v2);
			glm::vec3 v20 = glm::normalize(v2 + v0);

			CHECK(glm::length(v01) > 0.00001f);
			CHECK(glm::length(v12) > 0.00001f);
			CHECK(glm::length(v20) > 0.00001f);

			newPositions[i01 * 3] = v01.x;
			newPositions[i01 * 3 + 1] = v01.y;
			newPositions[i01 * 3 + 2] = v01.z;

			newPositions[i12 * 3] = v12.x;
			newPositions[i12 * 3 + 1] = v12.y;
			newPositions[i12 * 3 + 2] = v12.z;

			newPositions[i20 * 3] = v20.x;
			newPositions[i20 * 3 + 1] = v20.y;
			newPositions[i20 * 3 + 2] = v20.z;

			newIndices[q++] = i0;
			newIndices[q++] = i01;
			newIndices[q++] = i20;

			newIndices[q++] = i1;
			newIndices[q++] = i12;
			newIndices[q++] = i01;

			newIndices[q++] = i12;
			newIndices[q++] = i2;
			newIndices[q++] = i20;

			newIndices[q++] = i01;
			newIndices[q++] = i12;
			newIndices[q++] = i20;

			p += 3;
		}

		CHECK(p == newNumVertices);
		CHECK(q == newNumTriangles * 3);

		// replace
		delete[] positionData;
		delete[] indexData;

		positionData = newPositions;
		indexData = newIndices;

		numVertices = newNumVertices;
		numTriangles = newNumTriangles;
	}

	void IcosahedronGeometry::uploadToGPU() {
		updatePositionData(positionData, numVertices * 3);
		updateIndexData(indexData, numTriangles * 3);
	}

}

namespace pathos {

	AnselSkyComponent::~AnselSkyComponent() {
		if (sphere) {
			delete sphere;
			sphere = nullptr;
		}
	}

	void AnselSkyComponent::initialize(GLuint inTextureID) {
		textureID = inTextureID;
		sphere = new IcosahedronGeometry(0);
	}

	void AnselSkyComponent::createRenderProxy(SceneProxy* scene) {
		if (!hasValidResources()) {
			scene->anselSky = nullptr;
			return;
		}

		AnselSkyProxy* proxy = ALLOC_RENDER_PROXY<AnselSkyProxy>(scene);
		proxy->sphere = sphere;
		proxy->textureID = textureID;

		scene->anselSky = proxy;
	}

}

namespace pathos {

	void AnselSkyActor::initialize(GLuint textureID) {
		component->initialize(textureID);
	}

}
