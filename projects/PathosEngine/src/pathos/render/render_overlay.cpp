#include "render_overlay.h"
#include "pathos/engine.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/overlay/overlaypass.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/brush.h"

#include "badger/assertion/assertion.h"

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

	void OverlayRenderer::renderOverlay(RenderCommandList& cmdList, class DisplayObject2D* inRoot) {
		SCOPED_DRAW_EVENT(Overlay);

		CHECK(cmdList.sceneRenderTargets);
		const uint32 sceneWidth = cmdList.sceneRenderTargets->sceneWidth;
		const uint32 sceneHeight = cmdList.sceneRenderTargets->sceneHeight;

		root = inRoot;
		CHECK(root && root->isRoot());

		cmdList.viewport(0, 0, sceneWidth, sceneHeight);
		cmdList.disable(GL_CULL_FACE);
		cmdList.disable(GL_DEPTH_TEST);

		CHECK(cmdList.sceneRenderTargets);
		calculateTransformNDC(sceneWidth, sceneHeight);

		render_recurse(cmdList, root, toNDC);

		cmdList.enable(GL_DEPTH_TEST);
		cmdList.enable(GL_CULL_FACE);

		root = nullptr;
	}

	void OverlayRenderer::render_recurse(RenderCommandList& cmdList, class DisplayObject2D* object, const Transform& transformAccum) {
		if (object->getVisible() == false) {
			return;
		}

		Transform accum(transformAccum.getMatrix() * object->getTransform().getMatrix());

		auto brush = object->getBrush();
		if (brush) {
			auto renderpass = brush->configure(this, accum);
			if (object->getGeometry() != nullptr) {
				renderpass->renderOverlay(cmdList, object, accum);
			}
		}
		
		for (DisplayObject2D* child : object->getChildren()) {
			render_recurse(cmdList, child, accum);
		}
	}

	void OverlayRenderer::calculateTransformNDC(uint16 sceneWidth, uint16 sceneHeight) {
		toNDC.identity();
		toNDC.appendScale(2.0f / sceneWidth, -2.0f / sceneHeight, 1.0f);
		toNDC.appendMove(-1.0f, 1.0f, 0.0f);
	}

}
