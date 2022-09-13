#include "display_object_proxy.h"
#include "pathos/text/text_geometry.h"

// OverlaySceneProxy
namespace pathos {

	static constexpr uint32 RENDER_PROXY_ALLOCATOR_BYTES = 8 * 1024 * 1024; // 8 MB

	OverlaySceneProxy::OverlaySceneProxy()
		: renderProxyAllocator(RENDER_PROXY_ALLOCATOR_BYTES)
	{
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
