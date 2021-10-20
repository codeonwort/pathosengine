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
		, cpuTime(0.0f)
		, gpuTime(0.0f)
		, cpuTimeLabel(nullptr)
		, gpuTimeLabel(nullptr)
	{
	}

	DebugOverlay::~DebugOverlay() {
		//
	}

	void DebugOverlay::initialize() {
		root = DisplayObject2D::createRoot();
		root->addChild(cpuTimeLabel = new Label(L"cpu: 0.0 ms"));
		root->addChild(gpuTimeLabel = new Label(L"gpu: 0.0 ms"));
	}

	void DebugOverlay::renderDebugOverlay(RenderCommandList& cmdList, int32 screenWidth, int32 screenHeight) {
		if (!enabled) {
			return;
		}

		cpuTimeLabel->setVisible(showFrameStat);
		gpuTimeLabel->setVisible(showFrameStat);
		if (showFrameStat) {
			const float newCpuTime = gEngine->getCPUTime();
			const float newGpuTime = gEngine->getGPUTime();
			cpuTime += 0.1f * (newCpuTime - cpuTime);
			gpuTime += 0.1f * (newGpuTime - gpuTime);

			wchar_t buffer[256];
			swprintf_s(buffer, L"cpu: %.2f ms", cpuTime);
			cpuTimeLabel->setText(buffer);
			swprintf_s(buffer, L"gpu: %.2f ms", gpuTime);
			gpuTimeLabel->setText(buffer);
			
			const float baseX = pathos::max(0.0f, (float)screenWidth - 150.0f);
			const float baseY = pathos::min(400.0f, (float)screenHeight);
			cpuTimeLabel->setX(baseX);
			cpuTimeLabel->setY(baseY);
			gpuTimeLabel->setX(baseX);
			gpuTimeLabel->setY(baseY + 20);
		}

		renderer->renderOverlay(cmdList, root);
	}

}
