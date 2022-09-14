#include "display_object_proxy.h"
#include "pathos/text/text_geometry.h"

// OverlaySceneProxy
namespace pathos {

	static constexpr uint32 RENDER_PROXY_ALLOCATOR_BYTES = 8 * 1024 * 1024; // 8 MB

	OverlaySceneProxy::OverlaySceneProxy(uint32 inViewportWidth, uint32 inViewportHeight)
		: renderProxyAllocator(RENDER_PROXY_ALLOCATOR_BYTES)
		, viewportWidth(inViewportWidth)
		, viewportHeight(inViewportHeight)
	{
	}

}

// DisplayObject2DProxy
namespace pathos {

	matrix4 DisplayObject2DProxy::getLocalTransform() {
		matrix4 M = glm::scale(matrix4(1.0f), vector3(scaleX, scaleY, 1.0f));
		M = glm::translate(M, vector3(x, y, 0.0f));
		return M;
	}

}

// LabelProxy
namespace pathos {

	bool LabelProxy::onRender(RenderCommandList* cmdList) {
		TextGeometry* G = static_cast<TextGeometry*>(geometry);
		G->configure(*cmdList, *fontDesc.cacheTexture, text);
		return true;
	}

}
