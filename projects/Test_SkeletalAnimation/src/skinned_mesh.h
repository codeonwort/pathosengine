#pragma once

#include "pathos/mesh/mesh.h"

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "assimp/anim.h"

#include "bone.h"
#include "animation.h"

#include <map>

namespace pathos {

	struct Node {
		std::string name;
		std::vector<Node*> children;
		glm::mat4 localTransform;
		aiNode* ai;
	};

	class SkinnedMesh : public Mesh {

	public:
		SkinnedMesh();
		SkinnedMesh(MeshGeometry* G, Material* M);

		void addBone(uint32_t geomIndex, const Bone& bone);

		void addAnimation(SkeletalAnimation* animation);
		void setRoot(Node* root);
		inline void setNodeGlobalTransform(std::string& name, glm::mat4& transform) {
			nodeTransformMapping[name] = transform;
		}

		// TODO: switch to hardware skinning
		void setInitialPositions(uint32_t geomIndex, const std::vector<float>& positions0);
		void updateSoftwareSkinning();
		void updateAnimation(std::string name, double progress);
		void updateAnimation(int index, double progress);
		void updateGlobalTransform();

		inline ModelTransform& getTransform() { return transform; }

	protected:
		ModelTransform transform;

		std::vector<std::vector<Bone>> boneMapping;
		std::vector<std::vector<float>> initialPositionsMapping;

		std::vector<SkeletalAnimation*> animations;
		std::map<std::string, Node*> nodeMapping;
		std::map<std::string, glm::mat4> nodeTransformMapping;
		Node* root;

		// TODO: switch to hardware skinning
		void updateBoneTransform();

		int translation_lower_bound(aiNodeAnim* channel, double time);
		int rotation_lower_bound(aiNodeAnim* channel, double time);
		int scale_lower_bound(aiNodeAnim* channel, double time);

		glm::vec3 interpolate(aiVectorKey* keys, int len, int i, double time);
		glm::quat interpolate(aiQuatKey* keys, int len, int i, double time);

	};

}