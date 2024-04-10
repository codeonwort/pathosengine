#include "malloc_emulator.h"

#include <vector>

#define DEBUG_MALLOC_EMULATOR 0

namespace pathos {

	MallocEmulator::~MallocEmulator() {
		if (root != nullptr) {
			cleanup();
		}
	}

	void MallocEmulator::initialize(uint64 totalBytes) {
		CHECK(root == nullptr);
		root = new Range{ 0, totalBytes, nullptr, nullptr, nullptr };
		remainingBytes = totalBytes;
		numAllocations = 1; // root node
	}

	uint64 MallocEmulator::allocate(uint64 bytes) {
		CHECK(root != nullptr && bytes > 0);

		std::vector<Range*> Q{ root };
		Range* node = nullptr;
		while (!Q.empty()) {
			Range* cand = Q[Q.size() - 1];
			Q.pop_back();
			if (cand->isResident() == false) {
				if (cand->hasChild()) {
					if (cand->right->isResident() == false && cand->right->bytes >= bytes) Q.push_back(cand->right);
					if (cand->left->isResident() == false && cand->left->bytes >= bytes) Q.push_back(cand->left);
				} else if (cand->bytes >= bytes) {
					node = cand;
					break;
				}
			}
		}
		CHECKF(node != nullptr, "Can't find node to allocate");

		// If node is bigger than requested then split.
		uint64 finalOffset;
		if (node->bytes > bytes) {
			node->left = new Range{ node->offset, bytes, nullptr, nullptr, node };
			node->left->markResident();
			node->right = new Range{ node->offset + bytes, node->bytes - bytes, nullptr, nullptr, node };
			numAllocations += 2;
			finalOffset = node->left->offset;
		} else {
			// If exact-fit then use the node.
			node->markResident();
			finalOffset = node->offset;
		}
		debugTopology();
		return finalOffset;
	}

	void MallocEmulator::deallocate(uint64 offset) {
		Range* node = root;
		// Find the node to delete.
		while (node != nullptr) {
			if (node->offset == offset) {
				if (node->isResident()) {
					break;
				} else {
					node = node->left;
				}
			} else if (node->offset < offset) {
				node = node->right;
			} else {
				CHECK_NO_ENTRY();
				return;
			}
		}

		CHECK(node != nullptr && node->parent != nullptr);

		node->evict();

		auto parent = node;
		while (parent != nullptr && parent->isResident() == false && parent->hasChild() == false && parent->parent != nullptr) {
			parent = merge(parent);
		}
		debugTopology();
	}

	void MallocEmulator::debugTopology() {
#if DEBUG_MALLOC_EMULATOR
		struct Temp { Range* node; uint64 offset; uint64 bytes; };
		std::vector<Temp> Q{ Temp{root, 0, root->bytes} };
		while (!Q.empty()) {
			Temp cand = Q[Q.size() - 1];
			Range* node = cand.node;
			Q.pop_back();
			CHECKF(node->offset == cand.offset && node->bytes == cand.bytes, "offset or bytes mismatch");
			if (node->isResident()) CHECKF(node->hasChild() == false, "resident can't have children");
			if (node->hasChild()) {
				CHECKF(node->isResident() == false, "parent can't be resident");
				Q.push_back(Temp{ node->left, node->offset, node->left->bytes });
				Q.push_back(Temp{ node->right, node->offset + node->left->bytes, node->bytes - node->left->bytes });
			}
		}
#endif
	}

	MallocEmulator::Range* MallocEmulator::merge(Range* node) {
		auto reparent = [](Range* parent, Range* left, Range* right) {
			parent->left = left;
			parent->right = right;
			if (left != nullptr) left->parent = parent;
			if (right != nullptr) right->parent = parent;
		};

		auto parent = node->parent;
		if (parent->left == node) {
			if (parent->right->hasChild()) {
				if (parent->right->left->isResident() == false) {
					// Both parent->left and parent->right->left are not resident. Merge them.
					Range* node2 = parent->right;
					reparent(parent, node2->left, node2->right);
					parent->left->offset = parent->offset;
					parent->left->bytes = parent->bytes - parent->right->bytes;
					delete node;
					delete node2;
					numAllocations -= 2;
					return parent;
				}
				return nullptr;
			} else {
				if (parent->right->isResident() == false) {
					// Both parent->left and parent->right are not resident. Merge them.
					Range* node2 = parent->right;
					reparent(parent, node2->left, node2->right);
					delete node;
					delete node2;
					numAllocations -= 2;
					return parent;
				}
				return nullptr;
			}
		} else {
			if (parent->left->hasChild()) {
				if (parent->left->right->isResident() == false) {
					// Both parent->right and parent->left->right are not resident. Merge them.
					Range* node2 = parent->left;
					reparent(parent, node2->left, node2->right);
					CHECK(parent->right->offset == parent->left->offset + parent->left->bytes);
					parent->right->bytes = parent->bytes - parent->left->bytes;
					delete node;
					delete node2;
					numAllocations -= 2;
					return parent;
				}
				return nullptr;
			} else {
				if (parent->left->isResident() == false) {
					// Both children of parent are not resident. Merge them.
					Range* node2 = parent->left;
					reparent(parent, node2->left, node2->right);
					delete node;
					delete node2;
					numAllocations -= 2;
					return parent;
				}
				return nullptr;
			}
		}
		debugTopology();
	}

	void MallocEmulator::cleanup() {
		if (root == nullptr) return;
		std::vector<Range*> Q{ root };
		while (!Q.empty()) {
			auto node = Q[Q.size() - 1];
			Q.pop_back();
			if (node->hasChild()) {
				Q.push_back(node->left);
				Q.push_back(node->right);
			}
			delete node;
		}
		root = nullptr;
	}

}
