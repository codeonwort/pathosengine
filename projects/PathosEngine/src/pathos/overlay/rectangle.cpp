#include "rectangle.h"

namespace pathos {

	Rectangle::Rectangle(float _width, float _height) {
		setSize(_width, _height);
		geom = new PlaneGeometry(1.0f, 1.0f);
	}

	Rectangle::~Rectangle() {
		delete geom;
	}

	void Rectangle::setSize(float _width, float _height) {
		assert(_width >= 0.0f && _height >= 0.0f);
		width = _width;
		height = _height;
	}

	void Rectangle::updateTransform() {
		transform.identity();
		transform.appendMove(0.5f, 0.5f, 0.0f);
		transform.appendScale(width * scaleX, height * scaleY, 1.0f);
	}

}