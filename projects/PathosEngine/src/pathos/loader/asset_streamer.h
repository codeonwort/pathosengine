// Asynchronous load manager for various assets (images, 3d models, and so on)

#pragma once

#include "pathos/smart_pointer.h"

#include "badger/types/int_types.h"
#include "badger/system/thread_pool.h"

#include <list>
#include <mutex>
#include <vector>
#include <string>
#include <utility>
#include <functional>

namespace pathos {

	class AssetStreamer;
	class Material;
	class OBJLoader;
	class GLTFLoader;
	
	using WavefrontOBJHandler = std::function<void(OBJLoader* objLoader, uint64 payload)>;
	using GLTFHandler = std::function<void(GLTFLoader* loader, uint64 payload)>;

	// #todo-asset-streamer: Poor man's WTF delegates for asset streamer :(
	template<typename UserClass>
	using WavefrontOBJHandlerMethod = void (UserClass::*)(OBJLoader* loader, uint64 payload);
	template<typename UserClass>
	using GLTFHandlerMethod = void (UserClass::*)(GLTFLoader* loader, uint64 payload);

	// CAUTION: Do not use
	void internal_loadWavefrontOBJ(const WorkItemParam* param);
	void internal_loadGLTF(const WorkItemParam* param);

	//////////////////////////////////////////////////////////////////////////
	// Asset reference

	struct AssetReferenceWavefrontOBJ
	{
		AssetReferenceWavefrontOBJ(const char* inFilepath, const char* inBaseDir = "")
			: filepath(inFilepath)
			, baseDir(inBaseDir)
		{
		}
		void addMaterialOverride(const std::string& mtlName, assetPtr<Material> newMaterial) {
			for (auto& v : materialOverrides) {
				if (v.first == mtlName) {
					v.second = newMaterial;
					return;
				}
			}
			materialOverrides.push_back(std::make_pair(mtlName, newMaterial));
		}

		std::string filepath;
		std::string baseDir;
		std::vector<std::pair<std::string, assetPtr<Material>>> materialOverrides;
	};

	struct AssetReferenceGLTF {
		AssetReferenceGLTF(const char* inFilepath)
			: filepath(inFilepath)
		{}

		std::string filepath;
	};

	//////////////////////////////////////////////////////////////////////////
	// Load info

	struct AssetLoadInfoBase {
		class DummyType {};

		virtual ~AssetLoadInfoBase() {}
	};

	struct AssetLoadInfoBase_WavefrontOBJ : public AssetLoadInfoBase {
		virtual void invokeHandler() {}

		AssetStreamer* streamer;
		OBJLoader* loader;
		std::string filepath;
		std::string mtlDir;
		std::vector<std::pair<std::string, assetPtr<Material>>> materialOverrides;

		uint64 payload;
	};

	template<typename UserClass>
	struct AssetLoadInfo_WavefrontOBJ : AssetLoadInfoBase_WavefrontOBJ
	{
		UserClass* handlerOwner = nullptr;
		WavefrontOBJHandlerMethod<UserClass> handlerMethod = nullptr; // Only this is valid if handlerOwner is not null
		WavefrontOBJHandler handler; // Only this is valid if handlerOwner is null

		void invokeHandler() override {
			if (handlerOwner == nullptr) {
				handler(loader, payload);
			} else {
				(handlerOwner->*handlerMethod)(loader, payload);
			}
		}
	};

	struct AssetLoadInfoBase_GLTF : public AssetLoadInfoBase {
		virtual void invokeHandler() {}

		AssetStreamer* streamer;
		GLTFLoader* loader;
		std::string filepath;

		uint64 payload;
	};

	template<typename UserClass>
	struct AssetLoadInfo_GLTF : AssetLoadInfoBase_GLTF {
		UserClass* handlerOwner = nullptr;
		GLTFHandlerMethod<UserClass> handlerMethod = nullptr;
		GLTFHandler handler;

		void invokeHandler() override {
			if (handlerOwner == nullptr) {
				handler(loader, payload);
			} else {
				(handlerOwner->*handlerMethod)(loader, payload);
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

		void enqueueWavefrontOBJ(const char* inFilepath, const char* inBaseDir, WavefrontOBJHandler handler, uint64 payload);

		/// <summary>
		/// Request a Wavefront OBJ file and register a callback for load complete event. Use when the callback is a class method.
		/// You need to release the loader manually by releaseOBJLoader() or it will be alive until process termination.
		/// </summary>
		/// <typeparam name="UserClass">Class type that declares the callback method.</typeparam>
		/// <param name="assetRef">Asset to load.</param>
		/// <param name="handlerOwner">Class instance that will receive the event.</param>
		/// <param name="handlerMethod">Class method to use as a callback.</param>
		/// <param name="payload">A value that will be passed as an argument to the callback.</param>
		template<typename UserClass>
		void enqueueWavefrontOBJ(const AssetReferenceWavefrontOBJ& assetRef, UserClass* handlerOwner, WavefrontOBJHandlerMethod<UserClass> handlerMethod, uint64 payload);

		void enqueueGLTF(const char* inFilepath, GLTFHandler handler, uint64 payload);

		/// <summary>
		/// Request a GLTF file and register a callback for load complete event. Use when the callback is a class method.
		/// You need to release the loader manually by releaseGLTFLoader() or it will be alive until process termination.
		/// </summary>
		/// <typeparam name="UserClass">Class type that declares the callback method.</typeparam>
		/// <param name="assetRef">Asset to load.</param>
		/// <param name="handlerOwner">Class instance that will receive the event.</param>
		/// <param name="handlerMethod">Class method to use as a callback.</param>
		/// <param name="payload">A value that will be passed as an argument to the callback.</param>
		template<typename UserClass>
		void enqueueGLTF(const AssetReferenceGLTF& assetRef, UserClass* handlerOwner, GLTFHandlerMethod<UserClass> handlerMethod, uint64 payload);

		/// User should call this when they don't need the loader anymore, or it will be alive until process termination.
		void releaseOBJLoader(OBJLoader* loader);
		/// User should call this when they don't need the loader anymore, or it will be alive until process termination.
		void releaseGLTFLoader(GLTFLoader* loader);

	public:
		// Should be called in render thread
		void internal_flushLoadedAssets();
		// public due to thread pool callbacks.
		OBJLoader* internal_allocateOBJLoader();
		GLTFLoader* internal_allocateGLTFLoader();
		void internal_onLoaded_WavefrontOBJ(AssetLoadInfoBase_WavefrontOBJ* info);
		void internal_onLoaded_GLTF(AssetLoadInfoBase_GLTF* info);
		void internal_unregisterLoadInfo(AssetLoadInfoBase* info);

		// #todo-asset-streamer
		//void enqueueColladaDAE();
		//void enqueueImage();
		//void enqueueBlob();

	private:
		ThreadPool threadPool;
		std::list<AssetLoadInfoBase*> loadInfoList;

		std::list<OBJLoader*> objLoaderAllocator;
		std::list<GLTFLoader*> gltfLoaderAllocator;

		std::vector<AssetLoadInfoBase_WavefrontOBJ*> loadedOBJs;
		std::vector<AssetLoadInfoBase_GLTF*> loadedGLTFs;

		std::mutex mutex_loadedOBJs;
		std::mutex mutex_loadedGLTFs;
		
		std::list<OBJLoader*> objLoadersToDelete;
		std::list<GLTFLoader*> gltfLoadersToDelete;
	};

	template<typename UserClass>
	void AssetStreamer::enqueueWavefrontOBJ(
		const AssetReferenceWavefrontOBJ& assetRef,
		UserClass* handlerOwner,
		WavefrontOBJHandlerMethod<UserClass> handlerMethod,
		uint64 payload)
	{
		using LoadInfoType = AssetLoadInfo_WavefrontOBJ<UserClass>;
		LoadInfoType* arg = new LoadInfoType;
		loadInfoList.push_back(arg);

		arg->streamer = this;
		arg->filepath = assetRef.filepath;
		arg->mtlDir = assetRef.baseDir;
		arg->materialOverrides = assetRef.materialOverrides;

		arg->payload = payload;

		arg->handlerOwner = handlerOwner;
		arg->handlerMethod = handlerMethod;

		ThreadPoolWork work;
		work.arg = arg;
		work.routine = internal_loadWavefrontOBJ;

		threadPool.AddWorkSafe(work);
	}

	template<typename UserClass>
	void AssetStreamer::enqueueGLTF(
		const AssetReferenceGLTF& assetRef,
		UserClass* handlerOwner,
		GLTFHandlerMethod<UserClass> handlerMethod,
		uint64 payload)
	{
		using LoadInfoType = AssetLoadInfo_GLTF<UserClass>;
		LoadInfoType* arg = new LoadInfoType;
		loadInfoList.push_back(arg);

		arg->streamer = this;
		arg->filepath = assetRef.filepath;

		arg->handlerOwner = handlerOwner;
		arg->handlerMethod = handlerMethod;

		ThreadPoolWork work;
		work.arg = arg;
		work.routine = internal_loadGLTF;
		threadPool.AddWorkSafe(work);
	}

}
