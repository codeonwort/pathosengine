#include "display_object.h"
#include "display_object_proxy.h"
#include <assert.h>

namespace pathos {

	DisplayObject2D* DisplayObject2D::createRoot() {
		DisplayObject2D* root = new DisplayObject2D;
		root->root = root;
		return root;
	}

	DisplayObject2DProxy* DisplayObject2D::createRenderProxyHierarchy(
		DisplayObject2D* root,
		OverlaySceneProxy* overlaySceneProxy)
	{
		DisplayObject2DProxy* rootProxy = nullptr;

		if (root != nullptr) {
			rootProxy = root->createRenderProxy(overlaySceneProxy);
			if (rootProxy != nullptr) {
				for (DisplayObject2D* child : root->getChildren()) {
					DisplayObject2DProxy* childProxy = createRenderProxyHierarchy(child, overlaySceneProxy);
					if (childProxy != nullptr) {
						rootProxy->children.push_back(childProxy);
					}
				}
			}
		}

		return rootProxy;
	}

	DisplayObject2D::DisplayObject2D() {
		transform.identity();
	}

	DisplayObject2D::~DisplayObject2D() {}

	DisplayObject2DProxy* DisplayObject2D::createRenderProxy(OverlaySceneProxy* sceneProxy) {
		// Just contains child proxies to makeup the hierarchy.
		if (getVisible()) {
			DisplayObject2DProxy* proxy = sceneProxy->allocate<DisplayObject2DProxy>();
			proxy->x = x;
			proxy->y = y;
			proxy->scaleX = scaleX;
			proxy->scaleY = scaleY;
			return proxy;
		}
		return nullptr;
	}

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
		child->setRoot(getRoot());

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

	void DisplayObject2D::handleMouseLeftClick(int32 mouseX, int32 mouseY) {
		bool bHandled = false;
		if (mouseHitTest(mouseX, mouseY)) {
			if (onMouseClick != nullptr) {
				onMouseClick(mouseX, mouseY);
				bHandled = true;
			}
		}
		if (!bHandled || !bStopInputPropagation) {
			// #todo-input: Pass both local and global mouse positions
			int32 localMouseX = mouseX - (int32)getX();
			int32 localMouseY = mouseY - (int32)getY();
			for (DisplayObject2D* child : children) {
				child->handleMouseLeftClick(localMouseX, localMouseY);
			}
		}
	}

	void DisplayObject2D::handleMouseDrag(int32 mouseX, int32 mouseY) {
		bool bHandled = false;
		if (mouseHitTest(mouseX, mouseY)) {
			if (onMouseDrag != nullptr) {
				onMouseDrag(mouseX, mouseY);
				bHandled = true;
			}
		}
		if (!bHandled || !bStopInputPropagation) {
			// #todo-input: Pass both local and global mouse positions
			int32 localMouseX = mouseX - (int32)getX();
			int32 localMouseY = mouseY - (int32)getY();
			for (DisplayObject2D* child : children) {
				child->handleMouseDrag(localMouseX, localMouseY);
			}
		}
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
