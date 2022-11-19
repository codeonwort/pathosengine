#include "csm_debugger.h"
#include "pathos/mesh/mesh.h"
#include "pathos/scene/camera.h"
#include "pathos/util/math_lib.h"

// #todo-shadow: Make it variable?
constexpr uint32 NUM_CSM_FRUSTUMS = 4;

CSMDebugger::CSMDebugger() {
	setStaticMesh(new Mesh);
	getStaticMeshComponent()->castsShadow = false;

	// For camera frustum
	G = new ProceduralGeometry;
	Material* M = Material::createMaterialInstance("unlit");
	M->setConstantParameter("color", vector3(0.7f));
	M->bWireframe = true;
	getStaticMesh()->add(G, M);

	// For bounds of light view projections
	G2 = new ProceduralGeometry;
	Material* M2 = Material::createMaterialInstance("unlit");
	M2->setConstantParameter("color", vector3(0.7f, 0.0f, 0.0f));
	M2->bWireframe = true;
	getStaticMesh()->add(G2, M2);
}

void CSMDebugger::drawCameraFrustum(const Camera& camera, const vector3& sunDirection) {
	std::vector<vector3> frustumPlanes;
	camera.getFrustumVertices(frustumPlanes, NUM_CSM_FRUSTUMS);

	bool cascadeMasks[4] = { true, true, true, true };

	// Wireframe for camera frustum
	{
		G->clear();

		constexpr uint32 iMax = 4 * (NUM_CSM_FRUSTUMS + 1);
		for (uint32 i = 0; i < iMax; i += 4) {
			if (cascadeMasks[i / 4] == false || (i / 4 == 4 && cascadeMasks[3] == false)) {
				continue;
			}

			vector3 p0 = frustumPlanes[i + 0];
			vector3 p1 = frustumPlanes[i + 1];
			vector3 p2 = frustumPlanes[i + 2];
			vector3 p3 = frustumPlanes[i + 3];
			G->addTriangle(p0, p1, p2);
			G->addTriangle(p1, p2, p3);

			if (i < iMax - 4 && (i / 4 != 4 || cascadeMasks[i / 4 - 1])) {
				vector3 p4 = frustumPlanes[i + 4];
				vector3 p5 = frustumPlanes[i + 5];
				vector3 p6 = frustumPlanes[i + 6];
				vector3 p7 = frustumPlanes[i + 7];
				G->addQuad(p0, p4, p5, p1);
				G->addQuad(p2, p6, p7, p3);
				G->addQuad(p0, p2, p6, p4);
				G->addQuad(p1, p3, p7, p5);
			}
		}
		G->upload();
		G->calculateNormals();
		G->calculateTangentBasis();
	}

	// Wireframe for bounds of light view projections
	{
		G2->clear();

		auto calcBounds = [&sunDirection](const vector3* frustum, std::vector<vector3>& outVertices) -> void {
			vector3 sun_up, sun_right;
			pathos::calculateOrthonormalBasis(sunDirection, sun_up, sun_right);

			vector3 frustum_center(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				frustum_center += frustum[i];
			}
			frustum_center *= 0.125f;

			vector3 frustum_size(0.0f);
			for (int32 i = 0; i < 8; ++i) {
				vector3 delta = frustum[i] - frustum_center;
				frustum_size.x = pathos::max(frustum_size.x, fabs(glm::dot(delta, sun_right)));
				frustum_size.y = pathos::max(frustum_size.y, fabs(glm::dot(delta, sun_up)));
				frustum_size.z = pathos::max(frustum_size.z, fabs(glm::dot(delta, sunDirection)));
			}

			const vector3 signs[8] = {
				vector3(1,1,1), vector3(1,1,-1), vector3(1,-1,-1), vector3(1,-1,1),
				vector3(-1,1,1), vector3(-1,1,-1), vector3(-1,-1,-1), vector3(-1,-1,1)
			};
			for (int32 i = 0; i < 8; ++i) {
				vector3 s = signs[i] * frustum_size;
				vector3 d = (s.x * sun_right) + (s.y * sun_up) + (s.z * sunDirection);
				vector3 v = frustum_center + d;
				outVertices.push_back(v);
			}
		};

		std::vector<vector3> lightViewVertices;
		for (uint32 i = 0u; i <= NUM_CSM_FRUSTUMS; ++i) {
			calcBounds(&frustumPlanes[i * 4], lightViewVertices);
		}
		for (uint32 i = 0; i < (uint32)lightViewVertices.size(); i += 8) {
			if (cascadeMasks[i / 8] == false) {
				continue;
			}

			vector3 p0 = lightViewVertices[i + 0];
			vector3 p1 = lightViewVertices[i + 1];
			vector3 p2 = lightViewVertices[i + 2];
			vector3 p3 = lightViewVertices[i + 3];
			vector3 p4 = lightViewVertices[i + 4];
			vector3 p5 = lightViewVertices[i + 5];
			vector3 p6 = lightViewVertices[i + 6];
			vector3 p7 = lightViewVertices[i + 7];
			G2->addQuad(p0, p1, p2, p3);
			G2->addQuad(p4, p5, p6, p7);
			G2->addQuad(p0, p3, p7, p4);
			G2->addQuad(p0, p1, p5, p4);
			G2->addQuad(p2, p3, p7, p6);
			G2->addQuad(p1, p2, p6, p5);
		}
		G2->upload();
		G2->calculateNormals();
		G2->calculateTangentBasis();
	}
}
