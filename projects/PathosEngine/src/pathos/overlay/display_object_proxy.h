#pragma once

#include "pathos/util/transform_helper.h"
#include "pathos/text/font_mgr.h"

#include "badger/system/mem_alloc.h"
#include <vector>

namespace pathos {

	class MeshGeometry;
	class Brush;
	class RenderCommandList;

	// Render thread version of DisplayObject2D
	class DisplayObject2DProxy {
	public:
		virtual ~DisplayObject2DProxy() = default;

		// #todo-text: Temp callback. Configure anything for this object before rendering.
		//             Also, this object is skipped for rendering if false is returned.
		virtual bool onRender(RenderCommandList* cmdList) { return true; }

		matrix4 getLocalTransform();

		float x = 0.0f;
		float y = 0.0f;
		float scaleX = 1.0f;
		float scaleY = 1.0f;

		MeshGeometry* geometry = nullptr;
		Brush* brush = nullptr;
		Transform transform;
		std::vector<DisplayObject2DProxy*> children;
	};

	class LabelProxy : public DisplayObject2DProxy {
	public:
		bool onRender(RenderCommandList* cmdList) override;

		std::wstring text;
		FontDesc fontDesc;
	};

	class RectangleProxy : public DisplayObject2DProxy {
	public:
		float width;
		float height;
	};

	// Overlay version of SceneProxy
	class OverlaySceneProxy final {

	public:
		OverlaySceneProxy(uint32 inViewportWidth, uint32 inViewportHeight);

		template<typename T>
		T* allocate() {
			T* proxy = reinterpret_cast<T*>(renderProxyAllocator.alloc(sizeof(T)));
			CHECK(proxy != nullptr);
			new (proxy) T;
			return proxy;
		}

		StackAllocator renderProxyAllocator;

		uint32 viewportWidth;
		uint32 viewportHeight;

		DisplayObject2DProxy* appOverlayRootProxy = nullptr;
		DisplayObject2DProxy* debugOverlayRootProxy = nullptr;
		DisplayObject2DProxy* consoleWindowRootProxy = nullptr;
	};

}
