#pragma once

#include "pathos/named_object.h"
#include "pathos/mesh/geometry.h"
#include "pathos/wrapper/transform.h"
#include <vector>

namespace pathos {

	class Brush;
	class OverlaySceneProxy;
	class DisplayObject2DProxy;

	class DisplayObject2D : public NamedObject {

	public:
		static DisplayObject2D* createRoot();

		static DisplayObject2DProxy* createRenderProxyHierarchy(
			DisplayObject2D* root,
			OverlaySceneProxy* overlaySceneProxy);

	public:
		DisplayObject2D();
		virtual ~DisplayObject2D();

		virtual DisplayObject2DProxy* createRenderProxy(OverlaySceneProxy* sceneProxy);

		bool addChild(DisplayObject2D* child);
		bool removeChild(DisplayObject2D* child);
		inline const std::vector<DisplayObject2D*>& getChildren() { return children; }

		inline float getX() const { return x; }
		inline float getY() const { return y; }
		inline float getScaleX() const { return scaleX; }
		inline float getScaleY() const { return scaleY; }

		inline void setX(float value) { x = value; }
		inline void setY(float value) { y = value; }
		inline void setScaleX(float value) { scaleX = value; }
		inline void setScaleY(float value) { scaleY = value; }

		virtual MeshGeometry* getGeometry() { return nullptr; }
		inline const Transform& getTransform() {
			updateTransform();
			return transform;
		}

		inline bool isRoot() { return root == this; }
		inline DisplayObject2D* getRoot() { return root; }

		inline bool getVisible() { return visible; }
		inline void setVisible(bool value) { visible = value; }

		inline Brush* getBrush() { return brush; }
		inline void setBrush(Brush* newBrush) { brush = newBrush; }

	protected:
		float x = 0.0f, y = 0.0f;
		float scaleX = 1.0f, scaleY = 1.0f;

		Transform transform;
		bool transformDirty = false; // #todo-overlay: Utilize this

		void setRoot(DisplayObject2D* root);
		virtual void updateTransform() {}

	private:
		bool visible = true;
		DisplayObject2D* root = nullptr;
		DisplayObject2D* parent = nullptr;
		std::vector<DisplayObject2D*> children;
		Brush* brush = nullptr;

	};

}
