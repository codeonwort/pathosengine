#include "debug_overlay.h"
#include "pathos/engine.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/render/render_overlay.h"
#include "pathos/util/math_lib.h"

namespace pathos {

	constexpr int32 MAX_CYCLE_COUNTER_DISPLAY = 10;
	constexpr float CYCLE_COUNTER_SPACE_Y = 20.0f;

	DebugOverlay::DebugOverlay(OverlayRenderer* renderer2D)
		: renderer(renderer2D)
	{
	}

	DebugOverlay::~DebugOverlay() {
		//
	}

	void DebugOverlay::initialize() {
		root = uniquePtr<DisplayObject2D>(DisplayObject2D::createRoot());

		root->addChild(gameThreadTimeLabel = new Label(L"game thread: 0.0 ms"));
		root->addChild(renderThreadTimeLabel = new Label(L"render thread: 0.0 ms"));
		root->addChild(gpuTimeLabel = new Label(L"gpu: 0.0 ms"));
		
		cycleCounterBackground = new Rectangle(400.0f, MAX_CYCLE_COUNTER_DISPLAY * CYCLE_COUNTER_SPACE_Y);
		cycleCounterBackground->setBrush(new SolidColorBrush(0.1f, 0.1f, 0.1f));
		root->addChild(cycleCounterBackground);

		for (int32 i = 0; i < MAX_CYCLE_COUNTER_DISPLAY; ++i) {
			Label* nameLabel = new Label(L"name: ");
			nameLabel->setY(i * CYCLE_COUNTER_SPACE_Y);
			Label* valueLabel = new Label(L"0 ms");
			valueLabel->setX(300.0f);
			valueLabel->setY(i * CYCLE_COUNTER_SPACE_Y);
			cycleCounterBackground->addChild(nameLabel);
			cycleCounterBackground->addChild(valueLabel);
			cycleCounterNames.push_back(nameLabel);
			cycleCounterValues.push_back(valueLabel);
		}
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

		// #todo-stat:
		cycleCounterBackground->setVisible(showFrameStat);

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
			
			const float baseX = std::max(0.0f, (float)screenWidth - 220.0f);
			const float baseY = std::min(400.0f, (float)screenHeight);
			gameThreadTimeLabel->setX(baseX);
			gameThreadTimeLabel->setY(baseY);
			renderThreadTimeLabel->setX(baseX);
			renderThreadTimeLabel->setY(baseY + 20);
			gpuTimeLabel->setX(baseX);
			gpuTimeLabel->setY(baseY + 40);

			cycleCounterBackground->setX(std::max(0.0f, (float)screenWidth - 400.0f));
			cycleCounterBackground->setY(std::min(500.0f, (float)screenHeight));
		}

		renderer->renderOverlay(cmdList, rootProxy);
	}

}
