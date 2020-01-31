#include "asset_streamer.h"
#include "pathos/loader/objloader.h"

namespace pathos {

	static constexpr uint32 OBJ_POOL_SIZE = 32;

	void loadWavefrontOBJ(const WorkItemParam* param)
	{
		AssetLoadInfo_WavefrontOBJ* arg = (AssetLoadInfo_WavefrontOBJ*)param->arg;
		AssetStreamer* streamer = arg->streamer;

		OBJLoader* loader = streamer->objLoaderAllocator.alloc();
		CHECK(loader);

		arg->loader = loader;
		loader->load(arg->filepath, arg->mtlDir);

		streamer->mutex_loadedOBJs.lock();
		streamer->loadedOBJs.push_back(arg);
		streamer->mutex_loadedOBJs.unlock();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AssetStreamer::AssetStreamer()
		: objAllocator(PoolAllocator<AssetLoadInfo_WavefrontOBJ>(OBJ_POOL_SIZE))
		, objLoaderAllocator(PoolAllocator<OBJLoader>(OBJ_POOL_SIZE))
	{
	}

	AssetStreamer::~AssetStreamer()
	{
	}

	void AssetStreamer::initialize(uint32 numWorkerThreads)
	{
		threadPool.Start(numWorkerThreads);
	}

	void AssetStreamer::destroy()
	{
		threadPool.Stop();
	}

	void AssetStreamer::enqueueWavefrontOBJ(const char* inFilepath, const char* inMtlDir, WavefrontOBJHandler handler)
	{
		AssetLoadInfo_WavefrontOBJ* arg = objAllocator.alloc();
		CHECK(arg);

		arg->streamer = this;
		arg->filepath = inFilepath;
		arg->mtlDir = inMtlDir;
		arg->handler = handler;

		ThreadPoolWork work;
		work.arg = arg;
		work.routine = loadWavefrontOBJ;

		threadPool.AddWorkSafe(work);
	}

	void AssetStreamer::renderThread_flushLoadedAssets()
	{
		std::vector<AssetLoadInfo_WavefrontOBJ*> tempLoadedOBJs;

		// Handlers can take long time, so clone the array and release the mutex
		mutex_loadedOBJs.lock();
		tempLoadedOBJs = loadedOBJs;
		loadedOBJs.clear();
		mutex_loadedOBJs.unlock();

		for (AssetLoadInfo_WavefrontOBJ* assetInfo : tempLoadedOBJs) {
			assetInfo->handler(assetInfo->loader);
			assetInfo->streamer->objAllocator.dealloc(assetInfo);
		}
	}

}
