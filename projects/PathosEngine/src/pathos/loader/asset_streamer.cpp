#include "asset_streamer.h"
#include "pathos/loader/objloader.h"

namespace pathos {

	void loadWavefrontOBJ(const WorkItemParam* param)
	{
		AssetLoadInfo_WavefrontOBJ* arg = (AssetLoadInfo_WavefrontOBJ*)param->arg;

		OBJLoader loader(arg->filepath, arg->mtlDir);

		arg->handler(&loader);

		arg->streamer->objAllocator.dealloc(arg);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AssetStreamer::AssetStreamer()
		: objAllocator(PoolAllocator<AssetLoadInfo_WavefrontOBJ>(32))
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
		arg->streamer = this;
		arg->filepath = inFilepath;
		arg->mtlDir = inMtlDir;
		arg->handler = handler;

		ThreadPoolWork work;
		work.arg = arg;
		work.routine = loadWavefrontOBJ;

		threadPool.AddWorkSafe(work);
	}

}
