#pragma once

#include "display_object.h"
#include "pathos/mesh/geometry_primitive.h"

namespace pathos {

	class Rectangle : public DisplayObject2D {

	public:
		Rectangle(float width, float height);
		virtual ~Rectangle();

		void setSize(float width, float height);

		virtual MeshGeometry* getGeometry() override { return geom; }

	protected:
		float width, height;
		PlaneGeometry *geom = nullptr;

		virtual void updateTransform() override;

	};

}