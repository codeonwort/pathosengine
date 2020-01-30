// Asynchronous load manager for various assets (images, 3d models, and so on)

#pragma once

#include "badger/types/int_types.h"

#include <functional>

namespace pathos {

	using WavefrontOBJHandler = std::function<void(bool valid)>;

	class AssetStreamer final {

	public:
		AssetStreamer();
		~AssetStreamer();

		void initialize(uint32 numWorkerThreads);
		
		void destroy();

		void enqueueWavefrontOBJ(const char* inFilepath, const char* inBaseDir, WavefrontOBJHandler handler);

		// #todo-asset-streamer
		//void enqueueColladaDAE();
		//void enqueueImage();
		//void enqueueBlob();

	};

}
