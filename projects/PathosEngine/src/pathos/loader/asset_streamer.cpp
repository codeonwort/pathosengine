#include "asset_streamer.h"
#include "pathos/loader/objloader.h"
#include "pathos/util/cpu_profiler.h"
#include <sstream>

namespace pathos {

	static constexpr uint32 LOAD_INFO_STACK_SIZE = 4 * 1024; // 4 KB
	static constexpr uint32 OBJ_POOL_SIZE = 32;

	void internal_loadWavefrontOBJ(const WorkItemParam* param)
	{
		SCOPED_CPU_COUNTER(AsyncLoad_WavefrontOBJ);

		AssetLoadInfoBase_WavefrontOBJ* arg = (AssetLoadInfoBase_WavefrontOBJ*)param->arg;
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
		: loadInfoAllocator(StackAllocator(LOAD_INFO_STACK_SIZE))
		, objLoaderAllocator(PoolAllocator<OBJLoader>(OBJ_POOL_SIZE))
	{
	}

	AssetStreamer::~AssetStreamer()
	{
	}

	void AssetStreamer::initialize(uint32 numWorkerThreads)
	{
		threadPool.Start(numWorkerThreads);
		for (uint32 i = 0; i < numWorkerThreads; ++i) {
			std::stringstream ss;
			ss << "AssetStreamer_Worker " << i;

			uint32 threadId = threadPool.GetWorkerThreadId(i);
			CpuProfiler::getInstance().registerThread(threadId, ss.str().c_str());
		}
	}

	void AssetStreamer::destroy()
	{
		threadPool.Stop();
	}

	void AssetStreamer::enqueueWavefrontOBJ(const char* inFilepath, const char* inMtlDir, WavefrontOBJHandler handler)
	{
		using LoadInfoType = AssetLoadInfo_WavefrontOBJ<AssetLoadInfoBase::DummyType>;
		LoadInfoType* arg = reinterpret_cast<LoadInfoType*>(loadInfoAllocator.alloc(sizeof(LoadInfoType)));
		CHECKF(arg, "Out of memory for asset streamer load info");
		
		arg->streamer = this;
		arg->filepath = inFilepath;
		arg->mtlDir = inMtlDir;
		
		arg->handlerOwner = nullptr;
		arg->handler = handler;
		
		ThreadPoolWork work;
		work.arg = arg;
		work.routine = internal_loadWavefrontOBJ;
		
		threadPool.AddWorkSafe(work);
	}

	void AssetStreamer::renderThread_flushLoadedAssets()
	{
		std::vector<AssetLoadInfoBase_WavefrontOBJ*> tempLoadedOBJs;

		// Handlers can take long time, so clone the array and release the mutex
		mutex_loadedOBJs.lock();
		tempLoadedOBJs = loadedOBJs;
		loadedOBJs.clear();
		mutex_loadedOBJs.unlock();

		// #todo-renderthread-fatal: Now this is called in a real render thread,
		// but callbacks try to do TEMP_FLUSH_RENDER_COMMAND() and boom.
		for (AssetLoadInfoBase_WavefrontOBJ* assetInfo : tempLoadedOBJs) {
			assetInfo->invokeHandler();
		}
	}

}
