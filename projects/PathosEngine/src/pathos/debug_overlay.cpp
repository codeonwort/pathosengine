#include "debug_overlay.h"
#include "pathos/engine.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/display_object.h"
#include "pathos/render/render_overlay.h"
#include "pathos/util/math_lib.h"

namespace pathos {

	DebugOverlay::DebugOverlay(OverlayRenderer* renderer2D)
		: enabled(true)
		, showFrameStat(false)
		, renderer(renderer2D)
		, root(nullptr)
		, gameThreadTime(0.0f)
		, renderThreadTime(0.0f)
		, gpuTime(0.0f)
		, gameThreadTimeLabel(nullptr)
		, renderThreadTimeLabel(nullptr)
		, gpuTimeLabel(nullptr)
	{
	}

	DebugOverlay::~DebugOverlay() {
		//
	}

	void DebugOverlay::initialize() {
		root = DisplayObject2D::createRoot();
		root->addChild(gameThreadTimeLabel = new Label(L"game thread: 0.0 ms"));
		root->addChild(renderThreadTimeLabel = new Label(L"render thread: 0.0 ms"));
		root->addChild(gpuTimeLabel = new Label(L"gpu: 0.0 ms"));
	}

	void DebugOverlay::renderDebugOverlay(
		RenderCommandList& cmdList,
		DisplayObject2DProxy* rootProxy,
		int32 screenWidth,
		int32 screenHeight)
	{
		if (!enabled) {
			return;
		}

		gameThreadTimeLabel->setVisible(showFrameStat);
		renderThreadTimeLabel->setVisible(showFrameStat);
		gpuTimeLabel->setVisible(showFrameStat);
		if (showFrameStat) {
			const float newGameThreadTime = gEngine->getGameThreadCPUTime();
			const float newRenderThreadTime = gEngine->getRenderThreadCPUTime();
			const float newGpuTime = gEngine->getGPUTime();
			gameThreadTime += 0.1f * (newGameThreadTime - gameThreadTime);
			renderThreadTime += 0.1f * (newRenderThreadTime - renderThreadTime);
			gpuTime += 0.1f * (newGpuTime - gpuTime);

			wchar_t buffer[256];
			swprintf_s(buffer, L"game thread   : %.2f ms", gameThreadTime);
			gameThreadTimeLabel->setText(buffer);
			swprintf_s(buffer, L"render thread : %.2f ms", renderThreadTime);
			renderThreadTimeLabel->setText(buffer);
			swprintf_s(buffer, L"gpu           : %.2f ms", gpuTime);
			gpuTimeLabel->setText(buffer);
			
			const float baseX = pathos::max(0.0f, (float)screenWidth - 220.0f);
			const float baseY = pathos::min(400.0f, (float)screenHeight);
			gameThreadTimeLabel->setX(baseX);
			gameThreadTimeLabel->setY(baseY);
			renderThreadTimeLabel->setX(baseX);
			renderThreadTimeLabel->setY(baseY + 20);
			gpuTimeLabel->setX(baseX);
			gpuTimeLabel->setY(baseY + 40);
		}

		renderer->renderOverlay(cmdList, rootProxy);
	}

}
