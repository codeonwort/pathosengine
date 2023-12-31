#include "thread_pool.h"
#include "badger/system/cpu.h"
#include "badger/system/platform.h"
#include "badger/assertion/assertion.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

static void* PooledThreadMain(void* _param)
{
	PooledThreadParam* param = reinterpret_cast<PooledThreadParam*>(_param);
	int32 threadID           = param->threadID;
	ThreadPool* pool         = param->pool;

	wchar_t threadName[128];
	swprintf_s(threadName, L"AssetStreamer Worker %d", threadID);
	CPU::setCurrentThreadName(threadName);

	while (true)
	{
		if (pool->state == ThreadPoolState::PendingKill || pool->state == ThreadPoolState::Destroyed)
		{
			break;
		}

		ThreadPoolWork work;
		bool hasWork = pool->Internal_PopWork(work);

		if (!hasWork)
		{
			// #todo-thread-pool: cond_var could be unblocked spuriously (https://en.cppreference.com/w/cpp/thread/condition_variable/wait)
			std::unique_lock<std::mutex> cvLock(pool->worker_mutex);
			pool->cond_var.wait(cvLock);
		}
		else
		{
			WorkItemParam param;
			param.threadID = threadID;
			param.arg = work.arg;

			work.routine(&param);
		}
	}

	return 0;
}

void ThreadPool::Start(uint32 numWorkerThreads)
{
	CHECK(state == ThreadPoolState::NotStarted);
	CHECK(numWorkerThreads <= 256); // you sure it's not underflowed?

	if (state != ThreadPoolState::NotStarted)
	{
		return;
	}

	state = ThreadPoolState::Active;

	threads.resize(numWorkerThreads);
	threadParams.resize(numWorkerThreads);

	for (uint32 i = 0; i < numWorkerThreads; ++i)
	{
		threadParams[i].threadID = i;
		threadParams[i].pool = this;
	}

	for (uint32 i = 0; i < numWorkerThreads; ++i)
	{
		threads[i] = std::thread(PooledThreadMain, (void*)&threadParams[i]);
	}
}

void ThreadPool::Stop()
{
	CHECK(state == ThreadPoolState::Active);

	if (state != ThreadPoolState::Active)
	{
		return;
	}

	// Remove pending works
	{
		queueLock.lock();
		while (!queue.empty())
		{
			queue.pop();
		}
		queueLock.unlock();
	}

	// Wait for active works
	state = ThreadPoolState::PendingKill;
	cond_var.notify_all();
	WaitForAllWorks();

	state = ThreadPoolState::Destroyed;
}

void ThreadPool::WakeAllWorkers() {
	if (state == ThreadPoolState::Active) {
		cond_var.notify_all();
	}
}

void ThreadPool::WaitForAllWorks()
{
	CHECK(state == ThreadPoolState::Active || state == ThreadPoolState::PendingKill);

	if (state != ThreadPoolState::Active && state != ThreadPoolState::PendingKill)
	{
		return;
	}

	int32 n = (int32)threads.size();
	for (int32 i = 0; i < n; ++i)
	{
		if (threads[i].joinable())
		{
			threads[i].join();
		}
	}
}

void ThreadPool::AddWorkUnsafe(const ThreadPoolWork& workItem)
{
	queue.push(workItem);

	WakeAllWorkers();
}

void ThreadPool::AddWorkSafe(const ThreadPoolWork& workItem)
{
	queueLock.lock();
	queue.push(workItem);
	queueLock.unlock();

	WakeAllWorkers();
}

bool ThreadPool::Internal_PopWork(ThreadPoolWork& work)
{
	queueLock.lock();

	if (queue.empty())
	{
		queueLock.unlock();

		return false;
	}
	else
	{
		work = queue.front();
		queue.pop();
	}

	queueLock.unlock();

	return true;
}

uint32 ThreadPool::GetWorkerThreadId(uint32 workerThreadIndex)
{
#if PLATFORM_WINDOWS
	HANDLE handle = threads[workerThreadIndex].native_handle();
	DWORD id = GetThreadId(handle);
	return (uint32)id;
#else
	#error "Not implemented"
#endif
}
