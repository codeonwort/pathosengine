#pragma once

#include "pathos/mesh/geometry.h"
#include "pathos/util/transform_helper.h"

#include <vector>
#include <functional>

namespace pathos {

	class Brush;
	class OverlaySceneProxy;
	class DisplayObject2DProxy;

	namespace overlayInput {
		using OnMouseClick = std::function<void(int32 mouseX, int32 mouseY)>;
		using OnMouseDrag = std::function<void(int32 mouseX, int32 mouseY)>;
	}

	// Base class for 2D objects.
	class DisplayObject2D {

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

		inline bool isRoot() { return root == this; }
		inline DisplayObject2D* getRoot() { return root; }

	// Transform & visual appearance
	public:
		inline float getX() const { return x; }
		inline float getY() const { return y; }
		inline float getScaleX() const { return scaleX; }
		inline float getScaleY() const { return scaleY; }

		inline void setX(float value) { x = value; }
		inline void setY(float value) { y = value; }
		inline void setXY(float newX, float newY) { x = newX; y = newY; }
		inline void setScaleX(float value) { scaleX = value; }
		inline void setScaleY(float value) { scaleY = value; }

		virtual MeshGeometry* getGeometry() { return nullptr; }

		inline bool getVisible() { return bVisible; }
		inline void setVisible(bool value) { bVisible = value; }

		inline Brush* getBrush() { return brush; }
		inline void setBrush(Brush* newBrush) { brush = newBrush; }

	// User input
	public:
		void handleMouseLeftClick(int32 mouseX, int32 mouseY);
		void handleMouseDrag(int32 mouseX, int32 mouseY);

		bool mouseHitTest(int32 mouseX, int32 mouseY) const {
			if (bVisible == false || bReceivesMouseInput == false) return false;
			return onMouseHitTest(mouseX, mouseY);
		}

		virtual bool onMouseHitTest(int32 mouseX, int32 mouseY) const { return false; }

		// Application-provided callbacks
		overlayInput::OnMouseClick onMouseClick = nullptr;
		overlayInput::OnMouseDrag onMouseDrag = nullptr;

	public:
		bool bVisible = true;
		bool bReceivesMouseInput = true;
		bool bStopInputPropagation = false;

	protected:
		float x = 0.0f, y = 0.0f;
		float scaleX = 1.0f, scaleY = 1.0f;

		Transform transform;
		bool bTransformDirty = false; // #todo-overlay: Utilize this

		std::string displayName = "displayObject";
		void setDisplayName(const std::string& newName) { displayName = newName; }

		void setRoot(DisplayObject2D* root);
		virtual void updateTransform(uint32 viewportWidth, uint32 viewportHeight) {}

	private:
		DisplayObject2D* root = nullptr;
		DisplayObject2D* parent = nullptr;
		std::vector<DisplayObject2D*> children;

		Brush* brush = nullptr;

	};

}
