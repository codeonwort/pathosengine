#pragma once

#include "display_object.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	class Rectangle : public DisplayObject2D {

	public:
		Rectangle(float inWidth, float inHeight);
		virtual ~Rectangle();

		DisplayObject2DProxy* createRenderProxy(OverlaySceneProxy* sceneProxy) override;

		// User input
		virtual bool onMouseHitTest(int32 mouseX, int32 mouseY) const override;

		void setSize(float inWidth, float inHeight);

		virtual MeshGeometry* getGeometry() override { return geom; }

	protected:
		float width;
		float height;
		PlaneGeometry *geom = nullptr;

		virtual void updateTransform(uint32 viewportWidth, uint32 viewportHeight) override;

	};

}
