// Asynchronous load manager for various assets (images, 3d models, and so on)

#pragma once

#include "badger/types/int_types.h"
#include "badger/memory/mem_alloc.h"
#include "badger/thread/thread_pool.h"

#include <functional>

namespace pathos {

	class AssetStreamer;
	class OBJLoader;
	
	using WavefrontOBJHandler = std::function<void(OBJLoader * objLoader)>;

	struct AssetLoadInfo_WavefrontOBJ
	{
		AssetStreamer* streamer;

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

		// #todo-asset-streamer
		//void enqueueColladaDAE();
		//void enqueueImage();
		//void enqueueBlob();

	public:
		ThreadPool threadPool;
		PoolAllocator<AssetLoadInfo_WavefrontOBJ> objAllocator;

	};

}
