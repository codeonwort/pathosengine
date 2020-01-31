// Asynchronous load manager for various assets (images, 3d models, and so on)

#pragma once

#include "badger/types/int_types.h"
#include "badger/memory/mem_alloc.h"
#include "badger/thread/thread_pool.h"

#include <list>
#include <mutex>
#include <functional>

namespace pathos {

	class AssetStreamer;
	class OBJLoader;
	
	using WavefrontOBJHandler = std::function<void(OBJLoader* objLoader)>;

	struct AssetLoadInfo_WavefrontOBJ
	{
		AssetStreamer* streamer;
		OBJLoader* loader;

		const char* filepath;
		const char* mtlDir;
		WavefrontOBJHandler handler;
	};

	class AssetStreamer final {

	public:
		AssetStreamer();
		~AssetStreamer();

		void initialize(uint32 numWorkerThreads);
		
		void destroy();

		void enqueueWavefrontOBJ(const char* inFilepath, const char* inBaseDir, WavefrontOBJHandler handler);

		// Should be called in render thread
		void renderThread_flushLoadedAssets();

		// #todo-asset-streamer
		//void enqueueColladaDAE();
		//void enqueueImage();
		//void enqueueBlob();

	public:
		ThreadPool threadPool;
		PoolAllocator<AssetLoadInfo_WavefrontOBJ> objAllocator;
		PoolAllocator<OBJLoader> objLoaderAllocator;

		std::vector<AssetLoadInfo_WavefrontOBJ*> loadedOBJs;
		std::mutex mutex_loadedOBJs;

	};

}
