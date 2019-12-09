#include "rectangle.h"

namespace pathos {

	Rectangle::Rectangle(float inWidth, float inHeight) {
		setSize(inWidth, inHeight);
		geom = new PlaneGeometry(1.0f, 1.0f);
	}

	Rectangle::~Rectangle() {
		delete geom;
	}

	void Rectangle::setSize(float inWidth, float inHeight) {
		assert(inWidth >= 0.0f && inHeight >= 0.0f);
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
