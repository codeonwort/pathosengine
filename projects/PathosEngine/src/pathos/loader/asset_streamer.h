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

	// #todo-asset-streamer: poor man's WTF delegate for asset streamer :(
	template<typename UserClass>
	using WavefrontOBJHandlerMethod = void (UserClass::*)(OBJLoader* loader);

	// Do not use
	void internal_loadWavefrontOBJ(const WorkItemParam* param);

	//////////////////////////////////////////////////////////////////////////
	// asset reference

	struct AssetReferenceWavefrontOBJ
	{
		AssetReferenceWavefrontOBJ(const char* inFilepath, const char* inBaseDir = nullptr)
			: filepath(inFilepath)
			, baseDir(inBaseDir)
		{
		}
		const char* filepath;
		const char* baseDir;
	};

	//////////////////////////////////////////////////////////////////////////
	// load info

	struct AssetLoadInfoBase
	{
		class DummyType {};
	};

	struct AssetLoadInfoBase_WavefrontOBJ
	{
		virtual void invokeHandler() {}

		AssetStreamer* streamer;
		OBJLoader* loader;
		const char* filepath;
		const char* mtlDir;
	};

	template<typename UserClass>
	struct AssetLoadInfo_WavefrontOBJ : AssetLoadInfoBase_WavefrontOBJ
	{
		UserClass* handlerOwner = nullptr;
		WavefrontOBJHandlerMethod<UserClass> handlerMethod = nullptr; // Only this is valid if handlerOwner is not null
		WavefrontOBJHandler handler; // Only this is valid if handlerOwner is null

		void invokeHandler() override {
			if (handlerOwner == nullptr) {
				handler(loader);
			} else {
				(handlerOwner->*handlerMethod)(loader);
			}
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// AssetStreamer

	class AssetStreamer final {

	public:
		AssetStreamer();
		~AssetStreamer();

		void initialize(uint32 numWorkerThreads);
		
		void destroy();

		void wakeThreadPool();

		void enqueueWavefrontOBJ(const char* inFilepath, const char* inBaseDir, WavefrontOBJHandler handler);

		template<typename UserClass>
		void enqueueWavefrontOBJ(const AssetReferenceWavefrontOBJ& assetRef, UserClass* handlerOwner, WavefrontOBJHandlerMethod<UserClass> handlerMethod);

		// Should be called in render thread
		void flushLoadedAssets();

		// #todo-asset-streamer
		//void enqueueColladaDAE();
		//void enqueueImage();
		//void enqueueBlob();

	public:
		ThreadPool threadPool;
		StackAllocator loadInfoAllocator; // #todo-asset-streamer: Heck, this is even not recyclable.

		PoolAllocator<OBJLoader> objLoaderAllocator;
		std::vector<AssetLoadInfoBase_WavefrontOBJ*> loadedOBJs;
		std::mutex mutex_loadedOBJs;

	};

	template<typename UserClass>
	void AssetStreamer::enqueueWavefrontOBJ(const AssetReferenceWavefrontOBJ& assetRef, UserClass* handlerOwner, WavefrontOBJHandlerMethod<UserClass> handlerMethod)
	{
		using LoadInfoType = AssetLoadInfo_WavefrontOBJ<UserClass>;
		LoadInfoType* arg = reinterpret_cast<LoadInfoType*>(loadInfoAllocator.alloc(sizeof(LoadInfoType)));
		new (arg) LoadInfoType; // placement new to fill vtable
		CHECKF(arg, "Out of memory for asset streamer load info");

		arg->streamer = this;
		arg->filepath = assetRef.filepath;
		arg->mtlDir = assetRef.baseDir;

		arg->handlerOwner = handlerOwner;
		arg->handlerMethod = handlerMethod;

		ThreadPoolWork work;
		work.arg = arg;
		work.routine = internal_loadWavefrontOBJ;

		threadPool.AddWorkSafe(work);
	}

}
