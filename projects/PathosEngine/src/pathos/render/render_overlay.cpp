#include "render_overlay.h"
#include "pathos/engine.h"
#include "pathos/render/overlay/overlaypass.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/brush.h"

namespace pathos {

	OverlayRenderer::OverlayRenderer() {
		createShaders();
	}
	OverlayRenderer::~OverlayRenderer() {
		destroyShaders();
	}
	
	void OverlayRenderer::createShaders() {
		renderpass_standard = new OverlayPass_Standard;
		renderpass_text = new OverlayPass_Text;
	}
	void OverlayRenderer::destroyShaders() {
#define release(x) if(x) delete x
		release(renderpass_standard);
		release(renderpass_text);
#undef release
	}

	void OverlayRenderer::render(DisplayObject2D* root_) {
		assert(root_ && root_->isRoot());
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		calculateTransformNDC();
		root = root_;
		render_recurse(root, toNDC);
		root = nullptr;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

	void OverlayRenderer::render_recurse(DisplayObject2D* object, const Transform& transformAccum) {
		if (object->getVisible() == false) return;
		Transform accum(transformAccum.getMatrix() * object->getTransform().getMatrix());
		auto brush = object->getBrush();
		if (brush) {
			auto renderpass = brush->configure(this, accum);
			if (object->getGeometry() != nullptr) {
				renderpass->render(object, accum);
			}
		}
		
		for (DisplayObject2D* child : object->getChildren()) {
			render_recurse(child, accum);
		}
	}

	void OverlayRenderer::calculateTransformNDC() {
		// TODO: remove global access
		const auto& config = gEngine->getConfig();
		float width = static_cast<float>(config.windowWidth);
		float height = static_cast<float>(config.windowHeight);

		toNDC.identity();
		toNDC.appendScale(2.0f / width, -2.0f / height, 1.0f);
		toNDC.appendMove(-1.0f, 1.0f, 0.0f);
	}

}