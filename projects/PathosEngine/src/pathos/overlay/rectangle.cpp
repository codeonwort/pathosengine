#include "rectangle.h"
#include "display_object_proxy.h"
#include "badger/assertion/assertion.h"

namespace pathos {

	Rectangle::Rectangle(float inWidth, float inHeight) {
		setSize(inWidth, inHeight);
		geom = new PlaneGeometry(2.0f, 2.0f);
	}

	Rectangle::~Rectangle() {
		delete geom;
	}

	DisplayObject2DProxy* Rectangle::createRenderProxy(OverlaySceneProxy* sceneProxy) {
		if (getVisible()) {
			updateTransform(sceneProxy->viewportWidth, sceneProxy->viewportHeight);

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

	void Rectangle::updateTransform(uint32 viewportWidth, uint32 viewportHeight) {
		transform.identity();

		// Bring plane geometry that covers entire NDC to entire viewport
		transform.prependMove(1.0f, -1.0f, 0.0f);
		transform.prependScale(0.5f * viewportWidth, -0.5f * viewportHeight, 1.0f);

		transform.prependScale(width * scaleX / viewportWidth, height * scaleY / viewportHeight, 1.0f);
		transform.prependMove(x, y, 0.0f);
	}

}
