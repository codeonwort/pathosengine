#include "renderdoc_integration.h"
#include "log.h"
#include "pathos/engine.h"

#include <Windows.h>

namespace pathos {

	RenderDocIntegration& RenderDocIntegration::get()
	{
		static RenderDocIntegration instance;
		return instance;
	}

	RenderDocIntegration::RenderDocIntegration()
		: api(nullptr)
	{
		// ctor
	}

	RenderDocIntegration::~RenderDocIntegration()
	{
		// dtor
	}

	bool RenderDocIntegration::findInjectedDLL()
	{
	 	HMODULE dll = ::GetModuleHandleA("renderdoc.dll");
	 	if (dll == NULL) {
	 		LOG(LogError, "%s: Can't find renderdoc.dll from the process", __FUNCTION__);

			static bool first = true;
			if (first) {
				first = false;
				gEngine->registerConsoleCommand("find_renderdoc", [](const std::string& command) {
					RenderDocIntegration::get().findInjectedDLL();
				});
			}
			LOG(LogDebug, "Run 'find_renderdoc' to find injected dll later");

			return false;
	 	}

		// Versions in enum and log should match
		RENDERDOC_Version requested_api_version = eRENDERDOC_API_Version_1_4_1;
		LOG(LogInfo, "RenderDoc: Request API v1.4.1");

		pRENDERDOC_GetAPI apiEntry = (pRENDERDOC_GetAPI)::GetProcAddress(dll, "RENDERDOC_GetAPI");
		if (apiEntry == NULL) {
			LOG(LogError, "%s: Can't find api entry from renderdoc.dll", __FUNCTION__);
			return false;
		}

		int ret = apiEntry(requested_api_version, (void**)&api);
		if (ret != 1) {
			LOG(LogError, "%s: Can't get api functions from api entry", __FUNCTION__);
			api = nullptr;
			return false;
		}

		LOG(LogInfo, "RenderDoc integration is working with an injected dll");

		{
			int major, minor, patch;
			api->GetAPIVersion(&major, &minor, &patch);
			LOG(LogInfo, "RenderDoc: Actual API v%d.%d.%d", major, minor, patch);
		}

		gEngine->registerConsoleCommand("capture_frame", [](const std::string& command) {
			RenderDocIntegration::get().captureFrame();
		});

		return true;
	}

	void RenderDocIntegration::captureFrame()
	{
		if (api) {
			api->TriggerCapture();
		}
	}

}
