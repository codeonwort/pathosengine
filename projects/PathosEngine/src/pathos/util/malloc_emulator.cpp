#include "malloc_emulator.h"

#include <vector>

#define DEBUG_MALLOC_EMULATOR 0

static uint64 alignBytes(uint64 size, uint64 alignment) {
	return (size + (alignment - 1)) & ~(alignment - 1);
}

namespace pathos {

	MallocEmulator::~MallocEmulator() {
		cleanup();
	}

	void MallocEmulator::initialize(uint64 inTotalBytes, uint64 inAlignment) {
		alignment = inAlignment;
		CHECK(root == nullptr);
		root = new Range{ 0, inTotalBytes, nullptr, nullptr, nullptr };
		remainingBytes = inTotalBytes;
		numAllocations = 1; // root node
	}

	uint64 MallocEmulator::allocate(uint64 bytes) {
		CHECK(root != nullptr && bytes > 0);
		bytes = (alignment == 0) ? bytes : alignBytes(bytes, alignment);

		std::vector<Range*> Q{ root };
		Range* node = nullptr;
		// #todo-performance: Can't decide one branch and forget another unless I track 'maximum allocatable bytes' for every node.
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
		CHECKF(node != nullptr, "Can't find node to allocate"); // Out of memory, fragmentation, or bug.

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
				node = (node->right->offset <= offset) ? node->right : node->left;
			} else {
				CHECK_NO_ENTRY(); // Nothing was allocated at the given offset
				return;
			}
		}
		CHECK(node != nullptr && node->parent != nullptr);

		// Evict the node and merge empty nodes as much as possible.
		node->evict();
		while (node != nullptr && node->isResident() == false && node->hasChild() == false && node->parent != nullptr) {
			node = merge(node);
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
				CHECK(node->offset == node->left->offset);
				CHECK(node->left->parent == node);
				CHECK(node->right->parent == node);
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
					// Both parent->left and parent->right->left are not resident.
					// But if any left descendants of parent->right->left is resident,
					// we can't merge parent->left and parent->right->left.
					Range* leftist = parent->right->left;
					bool canMerge = true;
					while (leftist != nullptr) {
						leftist = leftist->left;
						if (leftist != nullptr && leftist->isResident()) {
							canMerge = false;
							break;
						}
					}
					// #todo-performance: Is there a case that nodes can be merged but I missed it?
					if (canMerge) {
						Range* node2 = parent->right;
						reparent(parent, node2->left, node2->right);
						leftist = parent->left;
						while (leftist != nullptr) {
							leftist->offset = leftist->parent->offset;
							leftist->bytes = leftist->parent->bytes - leftist->parent->right->bytes;
							leftist = leftist->left;
						}
						delete node;
						delete node2;
						numAllocations -= 2;
						return parent;
					}
					return nullptr;
				}
				return nullptr;
			} else {
				if (parent->right->isResident() == false) {
					// Both parent->left and parent->right are not resident and they have no children.
					// Safe to merge them.
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
					// Both parent->right and parent->left->right are not resident.
					// But if any right descendants of parent->left->right is resident,
					// we can't merge parent->right and parent->left->right.
					Range* rightest = parent->left->right;
					bool canMerge = true;
					while (rightest != nullptr) {
						rightest = rightest->right;
						if (rightest != nullptr && rightest->isResident()) {
							canMerge = false;
							break;
						}
					}
					// #todo-performance: Is there a case that nodes can be merged but I missed it?
					if (canMerge) {
						Range* node2 = parent->left;
						reparent(parent, node2->left, node2->right);
						rightest = parent->right;
						while (rightest != nullptr) {
							rightest->bytes = rightest->parent->bytes - rightest->parent->left->bytes;
							rightest = rightest->right;
						}
						delete node;
						delete node2;
						numAllocations -= 2;
						return parent;
					}
					return nullptr;
				}
				return nullptr;
			} else {
				if (parent->left->isResident() == false) {
					// Both children of parent are not resident and they have no children.
					// Safe to merge them.
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
