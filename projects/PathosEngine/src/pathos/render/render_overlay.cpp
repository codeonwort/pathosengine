#include "render_overlay.h"
#include "pathos/engine.h"
#include "pathos/render/scene_render_targets.h"
#include "pathos/render/overlay/overlaypass.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/display_object_proxy.h"
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

	void OverlayRenderer::renderOverlay(RenderCommandList& cmdList, DisplayObject2DProxy* inRootProxy) {
		SCOPED_DRAW_EVENT(Overlay);

		CHECK(cmdList.sceneRenderTargets);
		const uint32 sceneWidth = cmdList.sceneRenderTargets->unscaledSceneWidth;
		const uint32 sceneHeight = cmdList.sceneRenderTargets->unscaledSceneHeight;

		root = inRootProxy;
		//CHECK(root && root->isRoot());

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

	void OverlayRenderer::render_recurse(RenderCommandList& cmdList, DisplayObject2DProxy* object, const Transform& accumTransform) {
		Transform finalTransform = accumTransform;
		finalTransform.append(object->transform.getMatrix());

		if (object->brush != nullptr && object->geometry != nullptr) {
			OverlayPass* renderpass = object->brush->configure(this, finalTransform);
			if (object->onRender(&cmdList)) {
				renderpass->renderOverlay(cmdList, object, finalTransform);
			}
		}
		
		Transform newAccumTransform = accumTransform;
		newAccumTransform.append(object->getLocalTransform());
		for (DisplayObject2DProxy* child : object->children) {
			render_recurse(cmdList, child, newAccumTransform);
		}
	}

	void OverlayRenderer::calculateTransformNDC(uint32 sceneWidth, uint32 sceneHeight) {
		// From viewport convention to NDC
		// Viewport: (0,0) is top left, +x to right, +y to bottom
		toNDC.identity();
		toNDC.appendMove(-1.0f, 1.0f, 0.0f);
		toNDC.appendScale(2.0f / sceneWidth, -2.0f / sceneHeight, 1.0f);
	}

}
