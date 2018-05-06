#include "render_overlay.h"
#include "pathos/engine.h"

namespace pathos {

	OverlayRenderer::OverlayRenderer() {
		createShaders();
	}
	OverlayRenderer::~OverlayRenderer() {
		destroyShaders();
	}
	
	void OverlayRenderer::createShaders() {
		renderpass_standard = new OverlayPass_Standard;
	}
	void OverlayRenderer::destroyShaders() {
#define release(x) if(x) delete x
		release(renderpass_standard);
#undef release
	}

	void OverlayRenderer::render(DisplayObject2D* root_) {
		assert(root_ && root_->isRoot());
		calculateTransformNDC();
		root = root_;
		render_recurse(root, toNDC);
		root = nullptr;
	}

	void OverlayRenderer::render_recurse(DisplayObject2D* object, const Transform& transformAccum) {
		if (object->getVisible() == false) return;
		if (object->getGeometry() != nullptr) {
			renderpass_standard->render(object, transformAccum);
		}
		glm::mat4 mat = transformAccum.getMatrix() * object->getTransform().getMatrix();
		Transform accum(mat);
		for (DisplayObject2D* child : object->getChildren()) {
			// TODO: multiply object's transform
			render_recurse(child, accum);
		}
	}

	void OverlayRenderer::calculateTransformNDC() {
		// TODO: remove global access
		auto& config = Engine::getConfig();
		unsigned int width = config.width;
		unsigned int height = config.height;

		toNDC.identity();
		//toNDC.appendMove(-width / 2.0f, -height / 2.0f, 0.0f);
		toNDC.appendMove(-1.0f, 1.0f, 0.0f);
		toNDC.appendScale(4.0f / width, 4.0f / height, 1.0f);
	}

}