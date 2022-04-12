#include "gl_debug_group.h"
#include "pathos/render/render_device.h"

namespace pathos {

	DebugGroupMarker::DebugGroupMarker(class RenderCommandList* cmdList, const char* eventName) {
		uint32_t id = COMPILE_TIME_CRC32_STR(eventName);
		cmdList->pushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, -1, eventName);
		command_list = cmdList;
	}

	DebugGroupMarker::~DebugGroupMarker() {
		command_list->popDebugGroup();
	}

}

namespace pathos {

	const uint32                ScopedGpuCounter::MAX_GPU_COUNTERS = 16;
	bool                        ScopedGpuCounter::enable = true;

	bool                        ScopedGpuCounter::poolInitialized = false;
	bool                        ScopedGpuCounter::canBeginQuery = false;
	uint32                      ScopedGpuCounter::maxQueryObjects = 0;
	uint32                      ScopedGpuCounter::numUsedQueryObjects = 0;
	std::vector<GLuint>         ScopedGpuCounter::queryObjectPool;
	std::vector<std::string>    ScopedGpuCounter::queryCounterNames;

	ScopedGpuCounter::ScopedGpuCounter(class RenderCommandList* cmdList, const char* inCounterName)
		: command_list(cmdList)
		, counterName(inCounterName)
		, queryObject1(0)
		, queryObject2(0)
	{
		if (!ScopedGpuCounter::enable) {
			return;
		}

		bool validQueries = ScopedGpuCounter::getUnusedQueryObject(inCounterName, queryObject1, queryObject2);
		CHECKF(validQueries, "Failed to get a GL query object.");

		if (validQueries) {
			CHECKF(ScopedGpuCounter::canBeginQuery, "Another counter is already running! You cannot nest scoped counters.");
			cmdList->queryCounter(queryObject1, GL_TIMESTAMP);

			ScopedGpuCounter::canBeginQuery = false;
		}
	}

	ScopedGpuCounter::~ScopedGpuCounter() {
		if (!ScopedGpuCounter::enable) {
			return;
		}

		if (queryObject2 != 0) {
			command_list->queryCounter(queryObject2, GL_TIMESTAMP);

			ScopedGpuCounter::canBeginQuery = true;
		}
	}

	void ScopedGpuCounter::initializeQueryObjectPool(uint32 inMaxGpuCounters) {
		CHECKF(inMaxGpuCounters > 0, "inMaxGpuCounters should be greater than 0.");
		CHECKF(!poolInitialized, "Already initialized");

		poolInitialized = true;
		canBeginQuery = true;

		maxQueryObjects = inMaxGpuCounters * 2;
		numUsedQueryObjects = 0;

		queryObjectPool.resize(maxQueryObjects);
		queryCounterNames.resize(inMaxGpuCounters);

		gRenderDevice->createQueries(GL_TIMESTAMP, maxQueryObjects, queryObjectPool.data());
	}

	void ScopedGpuCounter::destroyQueryObjectPool() {
		CHECKF(poolInitialized, "Pool was not initialized");
		CHECKF(numUsedQueryObjects == 0, "There are queries not returned yet.");

		gRenderDevice->deleteQueries(maxQueryObjects, queryObjectPool.data());

		queryObjectPool.clear();
		queryCounterNames.clear();
		maxQueryObjects = 0;

		poolInitialized = false;
		canBeginQuery = false;
	}

	bool ScopedGpuCounter::getUnusedQueryObject(const char* inCounterName, GLuint& outQuery1, GLuint& outQuery2) {
		CHECKF(poolInitialized, "Pool was not initialized");
		CHECKF(numUsedQueryObjects < maxQueryObjects, "Starvation !!! Need to increase the pool size.");

		if (numUsedQueryObjects >= maxQueryObjects) {
			return false;
		}

		queryCounterNames[numUsedQueryObjects / 2] = inCounterName;
		outQuery1 = queryObjectPool[numUsedQueryObjects++];
		outQuery2 = queryObjectPool[numUsedQueryObjects++];

		return true;
	}

	uint32 ScopedGpuCounter::flushQueries(RenderCommandList& cmdList, std::vector<std::string>& outCounterNames, std::vector<float>& outElapsedMilliseconds) {
		CHECKF(poolInitialized, "Pool was not initialized");
		CHECK(isInRenderThread());

		outCounterNames.resize(numUsedQueryObjects / 2);
		outElapsedMilliseconds.resize(numUsedQueryObjects / 2);
		std::vector<GLuint64> beginTimeNSArray(numUsedQueryObjects / 2, 0U);
		std::vector<GLuint64> endTimeNSArray(numUsedQueryObjects / 2, 0U);
		
		for (uint32 i = 0; i < numUsedQueryObjects; i += 2) {
			cmdList.getQueryObjectui64v(queryObjectPool[i + 0], GL_QUERY_RESULT, &beginTimeNSArray[i / 2]);
			cmdList.getQueryObjectui64v(queryObjectPool[i + 1], GL_QUERY_RESULT, &endTimeNSArray[i / 2]);
		}
		cmdList.flushAllCommands();

		for (uint32 i = 0; i < numUsedQueryObjects; i += 2) {
			float elapsedMS = (float)((double)(endTimeNSArray[i / 2] - beginTimeNSArray[i / 2]) / 1000000.0);
			outCounterNames[i / 2] = queryCounterNames[i / 2];
			outElapsedMilliseconds[i / 2] = elapsedMS;
		}

		uint32 ret = numUsedQueryObjects / 2;
		numUsedQueryObjects = 0;

		return ret;
	}

}
