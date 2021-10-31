#pragma once

#include "pathos/named_object.h"
#include "pathos/mesh/geometry.h"
#include "pathos/wrapper/transform.h"
#include <vector>

namespace pathos {

	class DisplayObject2D : public NamedObject {

	public:
		static DisplayObject2D* createRoot();

		DisplayObject2D();
		virtual ~DisplayObject2D();

		// #todo-text: Temp callback. Configure anything for this object before rendering.
		//             Also, this object is skipped for rendering if false is returned.
		virtual bool onRender(RenderCommandList& cmdList) { return true; }

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
		inline const Transform& getTransform() { updateTransform(); return transform; }

		inline bool isRoot() { return root == this; }
		inline DisplayObject2D* getRoot() { return root; }

		inline bool getVisible() { return visible; }
		inline void setVisible(bool value) { visible = value; }

		inline class Brush* getBrush() { return brush; }
		inline void setBrush(class Brush* newBrush) { brush = newBrush; }

	protected:
		float x = 0.0f, y = 0.0f;
		float scaleX = 1.0f, scaleY = 1.0f;

		Transform transform;
		bool transformDirty = false; // TODO: utilize this

		void setRoot(DisplayObject2D* root);
		virtual void updateTransform() {}

	private:
		bool visible = true;
		DisplayObject2D* root = nullptr;
		DisplayObject2D* parent = nullptr;
		std::vector<DisplayObject2D*> children;
		class Brush* brush = nullptr;

	};

}