#include "rectangle.h"
#include "display_object_proxy.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	Rectangle::Rectangle(float inWidth, float inHeight) {
		setSize(inWidth, inHeight);
		geom = new PlaneGeometry(1.0f, 1.0f);
	}

	Rectangle::~Rectangle() {
		delete geom;
	}

	DisplayObject2DProxy* Rectangle::createRenderProxy(OverlaySceneProxy* sceneProxy) {
		if (getVisible()) {
			updateTransform();

			RectangleProxy* proxy = sceneProxy->allocate<RectangleProxy>();
			proxy->x = x;
			proxy->y = y;
			proxy->scaleX = scaleX;
			proxy->scaleY = scaleY;
			proxy->geometry = geom;
			proxy->brush = getBrush();
			proxy->transform = transform;
			proxy->width = width;
			proxy->height = height;
			return proxy;
		}
		return nullptr;
	}

	void Rectangle::setSize(float inWidth, float inHeight) {
		CHECK(inWidth >= 0.0f && inHeight >= 0.0f);
		width = inWidth;
		height = inHeight;
	}

	void Rectangle::updateTransform() {
		transform.identity();
		transform.appendMove(0.5f, 0.5f, 0.0f);
		transform.appendScale(width * scaleX, -height * scaleY, 1.0f);
		transform.appendMove(x, -y, 0.0f);
		transform.appendScale(1.0f, -1.0f, 1.0f);
	}

}
