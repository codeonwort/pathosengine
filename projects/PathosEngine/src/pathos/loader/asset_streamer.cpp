#include "asset_streamer.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/gltf_loader.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/thread/engine_thread.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include <sstream>

namespace pathos {

	static constexpr uint32 LOAD_INFO_STACK_SIZE = 4 * 1024; // 4 KB
	static constexpr uint32 OBJ_POOL_SIZE = 32;
	static constexpr uint32 GLTF_POOL_SIZE = 4;

	void internal_loadWavefrontOBJ(const WorkItemParam* param)
	{
		SCOPED_CPU_COUNTER(AsyncLoad_WavefrontOBJ);

		AssetLoadInfoBase_WavefrontOBJ* arg = (AssetLoadInfoBase_WavefrontOBJ*)param->arg;
		AssetStreamer* streamer = arg->streamer;

		OBJLoader* loader = streamer->objLoaderAllocator.alloc();
		CHECK(loader);

		arg->loader = loader;
		loader->setMaterialOverrides(std::move(arg->materialOverrides));
		bool bLoaded = loader->load(arg->filepath.c_str(), arg->mtlDir.c_str());

		// #todo-asset-streamer: How to inform load failure?
		if (bLoaded) {
			streamer->mutex_loadedOBJs.lock();
			streamer->loadedOBJs.push_back(arg);
			streamer->mutex_loadedOBJs.unlock();
		} else {
			LOG(LogError, "[AssetStreamer] Failed to load OBJ: %s", arg->filepath);
		}
	}

	void internal_loadGLTF(const WorkItemParam* param)
	{
		SCOPED_CPU_COUNTER(AsyncLoad_GLTF);

		AssetLoadInfoBase_GLTF* arg = (AssetLoadInfoBase_GLTF*)param->arg;
		AssetStreamer* streamer = arg->streamer;

		GLTFLoader* loader = streamer->gltfLoaderAllocator.alloc();
		CHECK(loader);

		arg->loader = loader;
		bool bLoaded = loader->loadASCII(arg->filepath.c_str());

		// #todo-asset-streamer: How to inform load failure?
		if (bLoaded) {
			streamer->mutex_loadedGLTFs.lock();
			streamer->loadedGLTFs.push_back(arg);
			streamer->mutex_loadedGLTFs.unlock();
		} else {
			LOG(LogError, "[AssetStreamer] Failed to load GLTF: %s", arg->filepath.c_str());
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AssetStreamer::AssetStreamer()
		: loadInfoAllocator(StackAllocator(LOAD_INFO_STACK_SIZE))
		, objLoaderAllocator(PoolAllocator<OBJLoader>(OBJ_POOL_SIZE))
		, gltfLoaderAllocator(PoolAllocator<GLTFLoader>(GLTF_POOL_SIZE))
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
		LOG(LogInfo, "[AssetStreamer] Destroy asset streamer");
	}

	void AssetStreamer::wakeThreadPool() {
		threadPool.WakeAllWorkers();
	}

	void AssetStreamer::enqueueWavefrontOBJ(const char* inFilepath, const char* inMtlDir, WavefrontOBJHandler handler, uint64 payload)
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

	void AssetStreamer::enqueueGLTF(const char* inFilepath, GLTFHandler handler, uint64 payload) {
		using LoadInfoType = AssetLoadInfo_GLTF<AssetLoadInfoBase::DummyType>;
		LoadInfoType* arg = reinterpret_cast<LoadInfoType*>(loadInfoAllocator.alloc(sizeof(LoadInfoType)));
		CHECKF(arg != nullptr, "Out of memory for asset streamer load info");

		arg->streamer = this;
		arg->filepath = inFilepath;

		arg->handlerOwner = nullptr;
		arg->handler = handler;

		ThreadPoolWork work;
		work.arg = arg;
		work.routine = internal_loadGLTF;
		threadPool.AddWorkSafe(work);
	}

	// #todo-renderthread: To execute or not to execute this on the render thread?
	void AssetStreamer::flushLoadedAssets()
	{
		CHECK(!isInRenderThread());
		std::vector<AssetLoadInfoBase_WavefrontOBJ*> tempLoadedOBJs;
		std::vector<AssetLoadInfoBase_GLTF*> tempLoadedGLTFs;

		// Handlers can take long time, so clone the array and release the mutex
		{
			mutex_loadedOBJs.lock();
			tempLoadedOBJs = loadedOBJs;
			loadedOBJs.clear();
			mutex_loadedOBJs.unlock();

			for (AssetLoadInfoBase_WavefrontOBJ* assetInfo : tempLoadedOBJs) {
				assetInfo->invokeHandler();
			}
		}
		{
			mutex_loadedGLTFs.lock();
			tempLoadedGLTFs = loadedGLTFs;
			loadedGLTFs.clear();
			mutex_loadedGLTFs.unlock();

			for (AssetLoadInfoBase_GLTF* assetInfo : tempLoadedGLTFs) {
				assetInfo->invokeHandler();
			}
		}
	}

}
