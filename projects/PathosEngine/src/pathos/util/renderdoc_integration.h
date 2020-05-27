#pragma once

#define WIN32
#include "renderdoc/api/app/renderdoc_app.h"

namespace pathos {

	class RenderDocIntegration {

	public:
		static RenderDocIntegration& get();

	private:
		RenderDocIntegration();
		~RenderDocIntegration();

		RenderDocIntegration(const RenderDocIntegration&)            = delete;
		RenderDocIntegration& operator=(const RenderDocIntegration&) = delete;

	public:
		bool findInjectedDLL();
		void captureFrame();

	private:
		RENDERDOC_API_1_4_1* api;

	};

}
