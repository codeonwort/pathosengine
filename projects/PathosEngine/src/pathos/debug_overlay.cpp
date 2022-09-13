#include "debug_overlay.h"
#include "pathos/engine.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/render/render_overlay.h"
#include "pathos/util/math_lib.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/string_conversion.h"
#include "pathos/thread/engine_thread.h"

#include <algorithm>

namespace pathos {

	constexpr float FRAME_STAT_BASE_Y = 400.0f;
	constexpr float FRAME_STAT_WIDTH = 220.0f;
	constexpr float FRAME_STAT_SPACE_Y = 20.0f;

	constexpr int32 MAX_CYCLE_COUNTER_DISPLAY = 10;
	constexpr float CYCLE_COUNTER_WIDTH = 400.0f;
	constexpr float CYCLE_COUNTER_BASE_Y = 500.0f;
	constexpr float CYCLE_COUNTER_LABEL_SPACE = 320.0f;
	constexpr float CYCLE_COUNTER_SPAN_X = 10.0f;
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
		
		cycleCounterBackground = new Rectangle(CYCLE_COUNTER_WIDTH, MAX_CYCLE_COUNTER_DISPLAY * CYCLE_COUNTER_SPACE_Y);
		cycleCounterBackground->setBrush(new SolidColorBrush(0.1f, 0.1f, 0.1f));
		root->addChild(cycleCounterBackground);

		cycleCounterGroupLabel = new Label(L"[main thread]");
		cycleCounterGroupLabel->setX(CYCLE_COUNTER_SPAN_X);
		cycleCounterBackground->addChild(cycleCounterGroupLabel);

		for (int32 i = 0; i < MAX_CYCLE_COUNTER_DISPLAY; ++i) {
			Label* nameLabel = new Label(L"name: ");
			nameLabel->setX(CYCLE_COUNTER_SPAN_X);
			nameLabel->setY((i + 1) * CYCLE_COUNTER_SPACE_Y);
			Label* valueLabel = new Label(L"0 ms");
			valueLabel->setX(CYCLE_COUNTER_LABEL_SPACE);
			valueLabel->setY((i + 1) * CYCLE_COUNTER_SPACE_Y);
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
			
			const float baseX = std::max(0.0f, (float)screenWidth - FRAME_STAT_WIDTH);
			const float baseY = std::min(FRAME_STAT_BASE_Y, (float)screenHeight);
			gameThreadTimeLabel->setX(baseX);
			gameThreadTimeLabel->setY(baseY);
			renderThreadTimeLabel->setX(baseX);
			renderThreadTimeLabel->setY(baseY + FRAME_STAT_SPACE_Y);
			gpuTimeLabel->setX(baseX);
			gpuTimeLabel->setY(baseY + 2.0f * FRAME_STAT_SPACE_Y);

			cycleCounterBackground->setX(std::max(0.0f, (float)screenWidth - CYCLE_COUNTER_WIDTH));
			cycleCounterBackground->setY(std::min(CYCLE_COUNTER_BASE_Y, (float)screenHeight));

			std::vector<ProfileItem> profileSnapshot;
			CpuProfiler::getInstance().getLastFrameSnapshot(pathos::gMainThreadId, profileSnapshot);
			std::sort(profileSnapshot.begin(), profileSnapshot.end(),
				[](const ProfileItem& x, const ProfileItem& y) {
					return x.elapsedMS > y.elapsedMS;
				});
			const int32 numValidCounters = std::min(MAX_CYCLE_COUNTER_DISPLAY, (int32)profileSnapshot.size());
			for (int32 i = 0; i < numValidCounters; ++i) {
				const ProfileItem& item = profileSnapshot[i];
				std::wstring wname;
				pathos::MBCS_TO_WCHAR(item.name, wname);
				wchar_t buffer[32];
				swprintf_s(buffer, L"%.3f ms", item.elapsedMS);

				cycleCounterNames[i]->setVisible(true);
				cycleCounterNames[i]->setText(wname.c_str());
				cycleCounterValues[i]->setVisible(true);
				cycleCounterValues[i]->setText(buffer);
			}
			for (int32 i = numValidCounters; i < MAX_CYCLE_COUNTER_DISPLAY; ++i) {
				cycleCounterNames[i]->setVisible(false);
				cycleCounterValues[i]->setVisible(false);
			}
		}

		renderer->renderOverlay(cmdList, rootProxy);
	}

}
