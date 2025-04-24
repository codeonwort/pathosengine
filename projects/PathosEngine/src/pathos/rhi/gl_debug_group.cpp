#include "gl_debug_group.h"
#include "pathos/rhi/render_device.h"
#include "pathos/rhi/render_command_list.h"

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

	const uint32      ScopedGpuCounter::MAX_GPU_COUNTERS = 256;
	GpuCounterContext ScopedGpuCounter::context;

	ScopedGpuCounter::ScopedGpuCounter(class RenderCommandList* cmdList, const char* inCounterName)
		: command_list(cmdList)
		, counterName(inCounterName)
		, queryObject1(0)
		, queryObject2(0)
	{
		CHECK(isInRenderThread());

		bool validQueries = ScopedGpuCounter::getUnusedQueryObject(inCounterName, ScopedGpuCounter::context.nested, queryObject1, queryObject2);
		CHECKF(validQueries, "Failed to get a GL query object.");

		if (validQueries) {
			cmdList->queryCounter(queryObject1, GL_TIMESTAMP);

			ScopedGpuCounter::context.nested += 1;
		}
	}

	ScopedGpuCounter::~ScopedGpuCounter() {
		CHECK(isInRenderThread());

		if (queryObject2 != 0) {
			command_list->queryCounter(queryObject2, GL_TIMESTAMP);

			ScopedGpuCounter::context.nested -= 1;
			CHECK(ScopedGpuCounter::context.nested >= 0);
		}
	}

	void ScopedGpuCounter::initializeQueryObjectPool(uint32 inMaxGpuCounters) {
		CHECKF(inMaxGpuCounters > 0, "inMaxGpuCounters should be greater than 0.");
		CHECKF(!context.bPoolInitialized, "Already initialized");

		context.bPoolInitialized = true;

		context.maxQueryObjects = inMaxGpuCounters * 2;
		context.numUsedQueryObjects = 0;

		context.queryObjectPool.resize(context.maxQueryObjects);
		context.queryCounterNames.resize(inMaxGpuCounters);
		context.indentLevels.resize(inMaxGpuCounters);

		gRenderDevice->createQueries(GL_TIMESTAMP, context.maxQueryObjects, context.queryObjectPool.data());
	}

	void ScopedGpuCounter::destroyQueryObjectPool() {
		CHECKF(context.bPoolInitialized, "Pool was not initialized");
		CHECKF(context.numUsedQueryObjects == 0, "There are queries not returned yet.");

		gRenderDevice->deleteQueries(context.maxQueryObjects, context.queryObjectPool.data());

		context.queryObjectPool.clear();
		context.queryCounterNames.clear();
		context.indentLevels.clear();
		context.maxQueryObjects = 0;

		context.bPoolInitialized = false;
	}

	bool ScopedGpuCounter::getUnusedQueryObject(const char* inCounterName, int32 nested, GLuint& outQuery1, GLuint& outQuery2) {
		CHECKF(context.bPoolInitialized, "Pool was not initialized");
		CHECKF(context.numUsedQueryObjects < context.maxQueryObjects, "Starvation !!! Need to increase the pool size.");

		if (context.numUsedQueryObjects >= context.maxQueryObjects) {
			return false;
		}

		context.queryCounterNames[context.numUsedQueryObjects / 2] = inCounterName;
		context.indentLevels[context.numUsedQueryObjects / 2] = nested;
		outQuery1 = context.queryObjectPool[context.numUsedQueryObjects++];
		outQuery2 = context.queryObjectPool[context.numUsedQueryObjects++];

		return true;
	}

	GpuCounterResult ScopedGpuCounter::flushQueries(RenderCommandList* cmdList) {
		CHECKF(context.bPoolInitialized, "Pool was not initialized");
		CHECK(isInRenderThread());

		GpuCounterResult result;
		result.numCounters = 0;

		if (context.numUsedQueryObjects == 0) {
			return result;
		}

		result.numCounters = context.numUsedQueryObjects / 2;
		result.counterNames.resize(result.numCounters);
		result.elapsedMilliseconds.resize(result.numCounters);
		result.indentLevels.resize(result.numCounters);

		std::vector<GLuint64> beginTimeNSArray(result.numCounters, 0U);
		std::vector<GLuint64> endTimeNSArray(result.numCounters, 0U);
		
		for (uint32 i = 0; i < context.numUsedQueryObjects; i += 2) {
			cmdList->getQueryObjectui64v(context.queryObjectPool[i + 0], GL_QUERY_RESULT, &beginTimeNSArray[i / 2]);
			cmdList->getQueryObjectui64v(context.queryObjectPool[i + 1], GL_QUERY_RESULT, &endTimeNSArray[i / 2]);
		}
		cmdList->flushAllCommands();

		for (uint32 i = 0; i < context.numUsedQueryObjects; i += 2) {
			float elapsedMS = (float)((double)(endTimeNSArray[i / 2] - beginTimeNSArray[i / 2]) / 1000000.0);
			result.counterNames[i / 2] = context.queryCounterNames[i / 2];
			result.elapsedMilliseconds[i / 2] = elapsedMS;
			result.indentLevels[i / 2] = context.indentLevels[i / 2];
		}

		context.numUsedQueryObjects = 0;

		return result;
	}

}
