#include "display_object.h"
#include <assert.h>

namespace pathos {

	DisplayObject2D* DisplayObject2D::createRoot() {
		DisplayObject2D* root = new DisplayObject2D;
		root->root = root;
		return root;
	}

	DisplayObject2D::DisplayObject2D() {
		transform.identity();
	}

	DisplayObject2D::~DisplayObject2D() {}

	bool DisplayObject2D::addChild(DisplayObject2D* child) {
		CHECK(child != nullptr && child->isRoot() == false);

		DisplayObject2D* obj = child->parent;
		while (obj) {
			if (obj == this) {
				// It's already child
				return false;
			}
			obj = obj->parent;
		}

		removeChild(child);
		children.push_back(child);

		child->parent = this;
		child->setRoot(this);

		return true;
	}

	bool DisplayObject2D::removeChild(DisplayObject2D* child) {
		CHECK(child != nullptr);

		auto it = std::find(children.begin(), children.end(), child);
		if (it != children.end()) {
			(*it)->parent = nullptr;
			(*it)->setRoot(nullptr);
			children.erase(it);

			return true;
		}

		return false;
	}

	void DisplayObject2D::setRoot(DisplayObject2D* newRoot) {
		if (root == newRoot) {
			return;
		}
		root = newRoot;

		for (auto child : children) {
			child->setRoot(newRoot);
		}
	}

}
