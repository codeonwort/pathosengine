#include "malloc_emulator.h"

#include <vector>

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
		numAllocations = 0;

		numAllocations += 1;
	}

	uint64 MallocEmulator::allocate(uint64 bytes) {
		CHECK(root != nullptr && bytes > 0);
		Range* node = root;
		while (node->hasChild()) {
			if (node->left->isResident() == false && node->left->bytes >= bytes) {
				node = node->left;
			} else if (node->right->isResident() == false && node->right->bytes >= bytes) {
				node = node->right;
			} else {
				return INVALID_OFFSET;
			}
		}
		if (node->bytes < bytes) {
			return INVALID_OFFSET;
		}

		// If node is bigger than requested then split.
		if (node->bytes > bytes) {
			node->left = new Range{ node->offset, bytes, nullptr, nullptr, node };
			node->left->markResident();
			node->right = new Range{ node->offset + bytes, node->bytes - bytes, nullptr, nullptr, node };
			numAllocations += 2;
			return node->left->offset;
		}
		// If exact-fit then use the node.
		node->markResident();
		return node->offset;
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
		while (parent != nullptr && parent->isResident() == false && parent->parent != nullptr) {
			parent = merge(parent);
		}
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
					reparent(parent, node->left, node->right);
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
					parent->right->offset = node2->offset;
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
					reparent(parent, node->left, node->right);
					delete node;
					delete node2;
					numAllocations -= 2;
					return parent;
				}
				return nullptr;
			}
		}
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
