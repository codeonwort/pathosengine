#include "asset_streamer.h"
#include "pathos/loader/objloader.h"
#include "pathos/loader/gltf_loader.h"
#include "pathos/rhi/render_device.h"
#include "pathos/util/cpu_profiler.h"
#include "pathos/util/engine_thread.h"
#include "pathos/util/log.h"

#include "badger/assertion/assertion.h"
#include <sstream>

namespace pathos {

	void internal_loadWavefrontOBJ(const WorkItemParam* param) {
		SCOPED_CPU_COUNTER(AsyncLoad_WavefrontOBJ);

		AssetLoadInfoBase_WavefrontOBJ* arg = (AssetLoadInfoBase_WavefrontOBJ*)param->arg;
		AssetStreamer* streamer = arg->streamer;

		streamer->internal_unregisterLoadInfo(arg);

		OBJLoader* loader = streamer->internal_allocateOBJLoader();

		arg->loader = loader;
		loader->setMaterialOverrides(std::move(arg->materialOverrides));
		bool bLoaded = loader->load(arg->filepath.c_str(), arg->mtlDir.c_str());

		if (bLoaded == false) {
			LOG(LogError, "[AssetStreamer] Failed to load OBJ: %s", arg->filepath);
		}
		// Enqueue anyway. You should check loader->isValid() in the callback.
		streamer->internal_onLoaded_WavefrontOBJ(arg);
	}

	void internal_loadGLTF(const WorkItemParam* param) {
		SCOPED_CPU_COUNTER(AsyncLoad_GLTF);

		AssetLoadInfoBase_GLTF* arg = (AssetLoadInfoBase_GLTF*)param->arg;
		AssetStreamer* streamer = arg->streamer;

		streamer->internal_unregisterLoadInfo(arg);

		GLTFLoader* loader = streamer->internal_allocateGLTFLoader();

		arg->loader = loader;
		bool bLoaded = loader->loadASCII(arg->filepath.c_str());

		if (bLoaded == false) {
			LOG(LogError, "[AssetStreamer] Failed to load GLTF: %s", arg->filepath.c_str());
		}
		// Enqueue anyway. You should check loader->isValid() in the callback.
		streamer->internal_onLoaded_GLTF(arg);
	}

	AssetLoadInfoBase_WavefrontOBJ::~AssetLoadInfoBase_WavefrontOBJ() {
		streamer->internal_destroyOBJLoader(loader);
	}

	AssetLoadInfoBase_GLTF::~AssetLoadInfoBase_GLTF() {
		streamer->internal_destroyGLTFLoader(loader);
	}

}

// AssetStreamer
namespace pathos {

	AssetStreamer::AssetStreamer() {
	}

	AssetStreamer::~AssetStreamer() {
		for (auto info : loadInfoList) {
			delete info;
		}
	}

	void AssetStreamer::initialize(uint32 numWorkerThreads) {
		threadPool.Start(numWorkerThreads);
		for (uint32 i = 0; i < numWorkerThreads; ++i) {
			std::stringstream ss;
			ss << "AssetStreamer_Worker " << i;

			uint32 threadId = threadPool.GetWorkerThreadId(i);
			CpuProfiler::getInstance().registerThread(threadId, ss.str().c_str());
		}
	}

	void AssetStreamer::destroy() {
		threadPool.Stop();
		LOG(LogInfo, "[AssetStreamer] Destroy asset streamer");
	}

	void AssetStreamer::wakeThreadPool() {
		threadPool.WakeAllWorkers();
	}

	void AssetStreamer::enqueueWavefrontOBJ(const char* inFilepath, const char* inMtlDir, WavefrontOBJHandler handler, uint64 payload) {
		using LoadInfoType = AssetLoadInfo_WavefrontOBJ<AssetLoadInfoBase::DummyType>;
		LoadInfoType* arg = new LoadInfoType;
		loadInfoList.push_back(arg);
		
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
		LoadInfoType* arg = new LoadInfoType;
		loadInfoList.push_back(arg);

		arg->streamer = this;
		arg->filepath = inFilepath;

		arg->handlerOwner = nullptr;
		arg->handler = handler;

		ThreadPoolWork work;
		work.arg = arg;
		work.routine = internal_loadGLTF;
		threadPool.AddWorkSafe(work);
	}

	void AssetStreamer::flushLoadedAssets() {
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
				delete assetInfo;
			}
		}
		{
			mutex_loadedGLTFs.lock();
			tempLoadedGLTFs = loadedGLTFs;
			loadedGLTFs.clear();
			mutex_loadedGLTFs.unlock();

			for (AssetLoadInfoBase_GLTF* assetInfo : tempLoadedGLTFs) {
				assetInfo->invokeHandler();
				delete assetInfo;
			}
		}
	}

	OBJLoader* AssetStreamer::internal_allocateOBJLoader() {
		auto loader = new OBJLoader;
		objLoaderAllocator.push_back(loader);
		return loader;
	}

	GLTFLoader* AssetStreamer::internal_allocateGLTFLoader() {
		auto loader = new GLTFLoader;
		gltfLoaderAllocator.push_back(loader);
		return loader;
	}

	void AssetStreamer::internal_destroyOBJLoader(OBJLoader* loader) {
		auto it = std::find(objLoaderAllocator.begin(), objLoaderAllocator.end(), loader);
		CHECK(it != objLoaderAllocator.end());
		objLoaderAllocator.erase(it);
		
		// #wip: When to delete loaders?
		//delete loader;
	}

	void AssetStreamer::internal_destroyGLTFLoader(GLTFLoader* loader) {
		auto it = std::find(gltfLoaderAllocator.begin(), gltfLoaderAllocator.end(), loader);
		CHECK(it != gltfLoaderAllocator.end());
		gltfLoaderAllocator.erase(it);

		// #wip: When to delete loaders?
		//delete loader;
	}

	void AssetStreamer::internal_onLoaded_WavefrontOBJ(AssetLoadInfoBase_WavefrontOBJ* info) {
		mutex_loadedOBJs.lock();
		loadedOBJs.push_back(info);
		mutex_loadedOBJs.unlock();
	}

	void AssetStreamer::internal_onLoaded_GLTF(AssetLoadInfoBase_GLTF* info) {
		mutex_loadedGLTFs.lock();
		loadedGLTFs.push_back(info);
		mutex_loadedGLTFs.unlock();
	}

	void AssetStreamer::internal_unregisterLoadInfo(AssetLoadInfoBase* info) {
		loadInfoList.remove(info);
	}

}
