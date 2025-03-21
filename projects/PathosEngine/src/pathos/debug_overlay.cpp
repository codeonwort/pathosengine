#include "debug_overlay.h"
#include "pathos/overlay/label.h"
#include "pathos/overlay/display_object.h"
#include "pathos/overlay/rectangle.h"
#include "pathos/overlay/brush.h"
#include "pathos/render/render_overlay.h"
#include "pathos/render/postprocessing/super_res.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/string_conversion.h"
#include "pathos/util/engine_thread.h"
#include "pathos/engine.h"

#include "badger/math/minmax.h"
#include <algorithm>
#include <vector>

namespace pathos {

	constexpr float STAT_UPDATE_INTERVAL         = 0.5f; // in seconds

	constexpr float FRAME_STAT_BASE_Y            = 400.0f;
	constexpr float FRAME_STAT_WIDTH             = 220.0f;
	constexpr float FRAME_STAT_SPACE_Y           = 20.0f;

	constexpr float CULL_STAT_BASE_Y             = 400.0f;
	constexpr float CULL_STAT_WIDTH              = 220.0f;
	constexpr float CULL_STAT_SPACE_Y            = 20.0f;

	constexpr int32 MAX_CPU_COUNTER_DISPLAY      = 10;
	constexpr int32 MAX_GPU_COUNTER_DISPLAY      = 12;
	constexpr float CYCLE_COUNTER_WIDTH          = 400.0f;
	constexpr float CYCLE_COUNTER_BASE_Y         = 500.0f;
	constexpr float CYCLE_COUNTER_LABEL_SPACE    = 320.0f;
	constexpr float CYCLE_COUNTER_SPAN_X         = 10.0f;
	constexpr float CYCLE_COUNTER_SPAN_Y         = 10.0f;
	constexpr float CYCLE_COUNTER_SPACE_Y        = 20.0f;

}

// CullStatList
namespace pathos {

	class CullStatList : public DisplayObject2D {
	public:
		CullStatList(float width, const wchar_t* title);
		void updateStat(uint32 total, uint32 culled);
	private:
		uniquePtr<Label> titleLabel, totalLabel, culledLabel, unculledLabel;
	};

	CullStatList::CullStatList(float spaceY, const wchar_t* title) {
		titleLabel = makeUnique<Label>(title);
		totalLabel = makeUnique<Label>();
		culledLabel = makeUnique<Label>();
		unculledLabel = makeUnique<Label>();
		totalLabel->setY(1.0f * spaceY);
		culledLabel->setY(2.0f * spaceY);
		unculledLabel->setY(3.0f * spaceY);
		addChild(titleLabel.get());
		addChild(totalLabel.get());
		addChild(culledLabel.get());
		addChild(unculledLabel.get());
	}

	void CullStatList::updateStat(uint32 total, uint32 culled) {
		wchar_t buffer[256];
		swprintf_s(buffer, L"total meshes    : %u", total);
		totalLabel->setText(buffer);
		swprintf_s(buffer, L"culled meshes   : %u", culled);
		culledLabel->setText(buffer);
		swprintf_s(buffer, L"unculled meshes : %u", total - culled);
		unculledLabel->setText(buffer);
	}

}

// CounterList
namespace pathos {

	class CounterList : public DisplayObject2D {
	public:
		CounterList(float width, uint32 maxItems, const wchar_t* groupLabelString);
		void updateCPUStat(std::vector<ProfileItem>& profileSnapshot);
		void updateGPUStat(const std::vector<std::string>& names, const std::vector<float>& times);
	private:
		int32 maxItems;
		uniquePtr<Rectangle> background;
		uniquePtr<Label> groupLabel;
		std::vector<uniquePtr<Label>> nameLabels;
		std::vector<uniquePtr<Label>> valueLabels;
	};

	CounterList::CounterList(float width, uint32 inMaxItems, const wchar_t* groupLabelString)
		: maxItems(inMaxItems)
	{
		background = makeUnique<Rectangle>(
			CYCLE_COUNTER_WIDTH,
			(1 + maxItems) * CYCLE_COUNTER_SPACE_Y + CYCLE_COUNTER_SPAN_Y);
		background->setBrush(new SolidColorBrush(0.1f, 0.1f, 0.1f));
		addChild(background.get());

		groupLabel = makeUnique<Label>(groupLabelString);
		groupLabel->setX(CYCLE_COUNTER_SPAN_X);
		background->addChild(groupLabel.get());

		for (int32 i = 0; i < maxItems; ++i) {
			uniquePtr<Label> nameLabel = makeUnique<Label>(L"name: ");
			nameLabel->setX(CYCLE_COUNTER_SPAN_X);
			nameLabel->setY((i + 1) * CYCLE_COUNTER_SPACE_Y);

			uniquePtr<Label> valueLabel = makeUnique<Label>(L"0 ms");
			valueLabel->setX(CYCLE_COUNTER_LABEL_SPACE);
			valueLabel->setY((i + 1) * CYCLE_COUNTER_SPACE_Y);

			background->addChild(nameLabel.get());
			background->addChild(valueLabel.get());
			nameLabels.push_back(std::move(nameLabel));
			valueLabels.push_back(std::move(valueLabel));
		}
	}

	void CounterList::updateCPUStat(std::vector<ProfileItem>& profileSnapshot) {
		std::sort(profileSnapshot.begin(), profileSnapshot.end(),
			[](const ProfileItem& x, const ProfileItem& y) {
				return x.elapsedMS > y.elapsedMS;
			});
		const int32 numValidCounters = badger::min(maxItems, (int32)profileSnapshot.size());
		for (int32 i = 0; i < numValidCounters; ++i) {
			const ProfileItem& item = profileSnapshot[i];
			std::wstring wname;
			pathos::MBCS_TO_WCHAR(item.name, wname);
			wchar_t buffer[32];
			swprintf_s(buffer, L"%.3f ms", item.elapsedMS);

			nameLabels[i]->setVisible(true);
			nameLabels[i]->setText(wname.c_str());
			valueLabels[i]->setVisible(true);
			valueLabels[i]->setText(buffer);
		}
		for (int32 i = numValidCounters; i < maxItems; ++i) {
			nameLabels[i]->setVisible(false);
			valueLabels[i]->setVisible(false);
		}
	}

	void CounterList::updateGPUStat(const std::vector<std::string>& names, const std::vector<float>& times) {
		const int32 numValidCounters = badger::min(maxItems, (int32)names.size());
		for (int32 i = 0; i < numValidCounters; ++i) {
			std::wstring wname;
			pathos::MBCS_TO_WCHAR(names[i], wname);
			wchar_t buffer[32];
			swprintf_s(buffer, L"%.3f ms", times[i]);

			nameLabels[i]->setVisible(true);
			nameLabels[i]->setText(wname.c_str());
			valueLabels[i]->setVisible(true);
			valueLabels[i]->setText(buffer);
		}
		for (int32 i = numValidCounters; i < maxItems; ++i) {
			nameLabels[i]->setVisible(false);
			valueLabels[i]->setVisible(false);
		}
	}

}

namespace pathos {

	DebugOverlay::DebugOverlay(OverlayRenderer* renderer2D)
		: renderer(renderer2D)
	{}

	DebugOverlay::~DebugOverlay() {}

	void DebugOverlay::initialize() {
		root = uniquePtr<DisplayObject2D>(DisplayObject2D::createRoot());

		root->addChild(gameThreadTimeLabel = new Label(L"game thread: 0.0 ms"));
		root->addChild(renderThreadTimeLabel = new Label(L"render thread: 0.0 ms"));
		root->addChild(gpuTimeLabel = new Label(L"gpu: 0.0 ms"));
		root->addChild(resolutionLabel = new Label(L"resolution: 1920x1080"));

		basePassCullList = new CullStatList(CULL_STAT_SPACE_Y, L"BasePass");
		root->addChild(basePassCullList);

		cpuCounterList = new CounterList(CYCLE_COUNTER_WIDTH, MAX_CPU_COUNTER_DISPLAY, L"[main thread]");
		root->addChild(cpuCounterList);

		gpuCounterList = new CounterList(CYCLE_COUNTER_WIDTH, MAX_GPU_COUNTER_DISPLAY, L"[gpu]");
		root->addChild(gpuCounterList);
	}

	void DebugOverlay::renderDebugOverlay(RenderCommandList& cmdList, DisplayObject2DProxy* rootProxy, int32 screenWidth, int32 screenHeight) {
		if (!bEnabled) {
			return;
		}

		gameThreadTimeLabel->setVisible(bShowFrameStat);
		renderThreadTimeLabel->setVisible(bShowFrameStat);
		gpuTimeLabel->setVisible(bShowFrameStat);
		resolutionLabel->setVisible(bShowFrameStat);

		basePassCullList->setVisible(bShowFrameStat);

		cpuCounterList->setVisible(bShowFrameStat);
		gpuCounterList->setVisible(bShowFrameStat);

		float currentWorldTime = gEngine->getWorldTime();
		bool bUpdateStat = currentWorldTime - lastStatUpdateTime > STAT_UPDATE_INTERVAL;

		if (bShowFrameStat && bUpdateStat) {
			lastStatUpdateTime = currentWorldTime;

			wchar_t buffer[256];

			// Frame time
			{
				const float newGameThreadTime = gEngine->getGameThreadCPUTime();
				const float newRenderThreadTime = gEngine->getRenderThreadCPUTime();
				const float newGpuTime = gEngine->getGPUTime();
				// Interpolation is not needed since bUpdateStat is introduced.
				//gameThreadTime += 0.1f * (newGameThreadTime - gameThreadTime);
				//renderThreadTime += 0.1f * (newRenderThreadTime - renderThreadTime);
				//gpuTime += 0.1f * (newGpuTime - gpuTime);
				gameThreadTime = newGameThreadTime;
				renderThreadTime = newRenderThreadTime;
				gpuTime = newGpuTime;

				swprintf_s(buffer, L"game thread   : %.2f ms", gameThreadTime);
				gameThreadTimeLabel->setText(buffer);
				swprintf_s(buffer, L"render thread : %.2f ms", renderThreadTime);
				renderThreadTimeLabel->setText(buffer);
				swprintf_s(buffer, L"gpu           : %.2f ms", gpuTime);
				gpuTimeLabel->setText(buffer);

				const float superResFactor = pathos::getSuperResolutionScaleFactor();
				if (superResFactor == 1.0f) {
					swprintf_s(buffer, L"resolution : %dx%d", screenWidth, screenHeight);
				} else {
					swprintf_s(buffer, L"resolution : %dx%d -> %dx%d",
						(int32)((float)screenWidth / superResFactor), (int32)((float)screenHeight / superResFactor),
						screenWidth, screenHeight);
				}
				resolutionLabel->setText(buffer);

				const float baseX = badger::max(0.0f, (float)screenWidth - FRAME_STAT_WIDTH);
				const float baseY = badger::min(FRAME_STAT_BASE_Y, (float)screenHeight);
				gameThreadTimeLabel->setXY(baseX, baseY);
				renderThreadTimeLabel->setXY(baseX, baseY + FRAME_STAT_SPACE_Y);
				gpuTimeLabel->setXY(baseX, baseY + 2.0f * FRAME_STAT_SPACE_Y);
				resolutionLabel->setXY(baseX - ((superResFactor == 1.0f) ? 0.0f : 100.0f), baseY + 3.0f * FRAME_STAT_SPACE_Y);
			}
			// Drawcall cull stat
			{
				const uint32 totalDrawcall = gEngine->internal_getLastBasePassTotalDrawcall();
				const uint32 culledMeshCount = gEngine->internal_getLastBasePassCulledDrawcall();
				basePassCullList->setX((float)screenWidth - FRAME_STAT_WIDTH - CULL_STAT_WIDTH);
				basePassCullList->setY(badger::min(CULL_STAT_BASE_Y, (float)screenHeight));
				basePassCullList->updateStat(totalDrawcall, culledMeshCount);
			}
			// Cycle counters
			{
				cpuCounterList->setX(badger::max(0.0f, (float)screenWidth - CYCLE_COUNTER_WIDTH));
				cpuCounterList->setY(badger::min(CYCLE_COUNTER_BASE_Y, (float)screenHeight));

				gpuCounterList->setX(badger::max(0.0f, (float)screenWidth - CYCLE_COUNTER_WIDTH));
				gpuCounterList->setY(badger::min(CYCLE_COUNTER_BASE_Y + CYCLE_COUNTER_SPACE_Y * (2 + MAX_GPU_COUNTER_DISPLAY), (float)screenHeight));

				std::vector<ProfileItem> profileSnapshot;
				CpuProfiler::getInstance().getLastFrameSnapshot(pathos::gMainThreadId, profileSnapshot);
				cpuCounterList->updateCPUStat(profileSnapshot);

				std::vector<std::string> gpuCounterNames;
				std::vector<float> gpuCounterTimes;
				gEngine->internal_getLastGPUCounters(gpuCounterNames, gpuCounterTimes);
				gpuCounterList->updateGPUStat(gpuCounterNames, gpuCounterTimes);
			}
		}

		renderer->renderOverlay(cmdList, rootProxy);
	}

}
