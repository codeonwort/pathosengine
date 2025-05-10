#include "skinned_mesh.h"

#include <glm/gtc/matrix_transform.hpp>

#include <functional>

namespace pathos {

	SkinnedMesh::SkinnedMesh() : StaticMesh() {}

	SkinnedMesh::SkinnedMesh(MeshGeometry* G, assetPtr<Material> M) : StaticMesh(G, M) {}

	void SkinnedMesh::addBone(uint32_t geomIndex, const Bone& bone) {
		if (boneMapping.size() <= geomIndex) {
			boneMapping.resize(geomIndex + 1);
		}
		boneMapping[geomIndex].push_back(bone);
	}

	void SkinnedMesh::addAnimation(SkeletalAnimation* animation) {
		animations.push_back(animation);
	}

	void SkinnedMesh::setRoot(Node* root) {
		assert(root);
		this->root = root;

		nodeMapping.clear();
		std::function<void(Node*)> recurse = [&](Node* node) {
			nodeMapping[node->name] = node;
			for (Node* child : node->children) {
				recurse(child);
			}
		};
		recurse(root);
	}

	// TODO: switch to hardware skinning
	void SkinnedMesh::setInitialPositions(uint32_t geomIndex, const std::vector<float>& positions0) {
		if (initialPositionsMapping.size() <= geomIndex) {
			initialPositionsMapping.resize(geomIndex + 1);
		}
		initialPositionsMapping[geomIndex] = positions0;
	}

	void SkinnedMesh::updateSoftwareSkinning() {
		updateGlobalTransform();
		updateBoneTransform();

		// #todo-lod
		const uint32 LOD = 0;
		const auto& geometries = getLOD(LOD).geometries;

		uint32_t geomIndex = 0;
		for (const auto G : geometries) {
			const auto& positions = initialPositionsMapping[geomIndex];
			std::vector<float> pos(positions.size(), 0.0f);
			const auto& bones = boneMapping[geomIndex];
			for (auto i = 0u; i < bones.size(); ++i) {
				const auto& bone = bones[i];
				for (auto j = 0u; j < bone.weights.size(); ++j) {
					auto p = 3 * bone.vertexIDs[j];
					auto vert = glm::vec4(positions[p], positions[p + 1], positions[p + 2], 1.0f);
					auto v = bone.weights[j] * bone.finalTransform * vert;
					pos[p] += v.x;
					pos[p + 1] += v.y;
					pos[p + 2] += v.z;
				}
			}
			G->updatePositionData(&pos[0], (uint32_t)pos.size());

			++geomIndex;
		}
	}
	void SkinnedMesh::updateAnimation(int index, double time) {
		aiAnimation* anim = animations[index]->anim;
		time = std::max(0.0, std::min(time, anim->mDuration));

		for (auto i = 0u; i < anim->mNumChannels; ++i) {
			aiNodeAnim* chan = anim->mChannels[i];
			int ix_trans = translation_lower_bound(chan, time);
			int ix_rot = rotation_lower_bound(chan, time);
			int ix_scale = scale_lower_bound(chan, time);

			auto p = interpolate(chan->mPositionKeys, chan->mNumPositionKeys, ix_trans, time);
			auto r = interpolate(chan->mRotationKeys, chan->mNumRotationKeys, ix_rot, time);
			auto s = interpolate(chan->mScalingKeys, chan->mNumScalingKeys, ix_scale, time);

			glm::mat4& T = glm::translate(glm::mat4(1.0f), p);
			glm::mat4& R = glm::toMat4(r);
			glm::mat4& S = glm::scale(glm::mat4(1.0f), s);

			std::string nodeName = chan->mNodeName.C_Str();
			Node* node = nodeMapping[nodeName];
			node->localTransform = T * R * S;
		}
	}
	void SkinnedMesh::updateAnimation(const std::string& name, double time) {
		// TODO: encalsulate aiAnimation
		for (auto i = 0u; i < animations.size(); ++i) {
			if (std::string(animations[i]->getName()) == name) {
				updateAnimation(i, time);
				break;
			}
		}
	}

	void SkinnedMesh::updateGlobalTransform() {
		// acquire transforms from the node hierarchy
		std::function<void(Node*, glm::mat4&)> calcTransform = [&](Node* node, glm::mat4& parent) {
			glm::mat4 T = parent * node->localTransform;
			setNodeGlobalTransform(node->name, T);
			for (auto i = 0u; i < node->children.size(); ++i) {
				calcTransform(node->children[i], T);
			}
		};
		calcTransform(root, glm::mat4(1.0f));
	}

	void SkinnedMesh::updateBoneTransform() {
		for (auto& bones : boneMapping) {
			for (auto& bone : bones) {
				bone.finalTransform = nodeTransformMapping[bone.name] * bone.offset;
			}
		}
	}

	int SkinnedMesh::translation_lower_bound(aiNodeAnim* channel, double time) {
		if (channel->mNumPositionKeys == 1) return 0;
		for (auto i = 0u; i < channel->mNumPositionKeys - 1; ++i) {
			if (time < channel->mPositionKeys[i + 1].mTime) {
				return i;
			}
		}
		abort();
		return -1;
	}

	int SkinnedMesh::rotation_lower_bound(aiNodeAnim* channel, double time) {
		if (channel->mNumRotationKeys == 1) return 0;
		for (auto i = 0u; i < channel->mNumRotationKeys - 1; ++i) {
			if (time < channel->mRotationKeys[i + 1].mTime) {
				return i;
			}
		}
		abort();
		return -1;
	}

	int SkinnedMesh::scale_lower_bound(aiNodeAnim* channel, double time) {
		if (channel->mNumScalingKeys == 1) return 0;
		for (auto i = 0u; i < channel->mNumScalingKeys - 1; ++i) {
			if (time < channel->mScalingKeys[i + 1].mTime) {
				return i;
			}
		}
		abort();
		return -1;
	}

	vector3 SkinnedMesh::interpolate(aiVectorKey* keys, int len, int i, double time) {
		aiVector3D result;
		if (len == 1) {
			result = keys[0].mValue;
		} else {
			float ratio = static_cast<float>((time - keys[i].mTime) / (keys[i + 1].mTime - keys[i].mTime));
			result = ratio * keys[i].mValue + (1 - ratio) * keys[i + 1].mValue;
		}
		return vector3(result.x, result.y, result.z);
	}
	glm::quat SkinnedMesh::interpolate(aiQuatKey* keys, int len, int i, double time) {
		aiQuaternion result;
		if (len == 1) {
			result = keys[0].mValue;
		} else {
			float ratio = static_cast<float>((time - keys[i].mTime) / (keys[i + 1].mTime - keys[i].mTime));
			aiQuaternion::Interpolate(result, keys[i].mValue, keys[i + 1].mValue, ratio);
			result = result.Normalize();
		}
		return glm::quat(result.w, result.x, result.y, result.z);
	}

}