#include "thread_pool.h"
#include "badger/thread/cpu.h"
#include "badger/system/platform.h"
#include "badger/assertion/assertion.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

static void* PooledThreadMain(void* _param)
{
	PooledThreadParam* param = reinterpret_cast<PooledThreadParam*>(_param);
	int32 workerIndex        = param->workerIndex;
	ThreadPool* pool         = param->pool;

	wchar_t threadName[128];
	swprintf_s(threadName, L"ThreadPool(%S) Worker %d", param->poolName.c_str(), workerIndex);
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
			pool->Internal_SetWorkerHasWork(workerIndex, false);
			pool->cond_var.wait(cvLock);
		}
		else
		{
			pool->Internal_SetWorkerHasWork(workerIndex, true);

			WorkItemParam param;
			param.threadIndex = workerIndex;
			param.arg = work.arg;

			work.routine(&param);
		}
	}

	return 0;
}

void ThreadPool::Start(const char* poolName, uint32 numWorkerThreads)
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
	hasWorkFlags.resize(numWorkerThreads, false);

	for (uint32 i = 0; i < numWorkerThreads; ++i)
	{
		threadParams[i].workerIndex = i;
		threadParams[i].pool = this;
		threadParams[i].poolName = poolName;
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
	WaitForWorkersTermination();

	state = ThreadPoolState::Destroyed;
}

void ThreadPool::WakeAllWorkers()
{
	if (state == ThreadPoolState::Active) {
		cond_var.notify_all();
	}
}

void ThreadPool::WaitForActiveWorks()
{
	bool bAnyWork = false;

	queueLock.lock();
	bAnyWork = queue.size() > 0;
	queueLock.unlock();

	hasWorkMutex.lock();
	for (size_t i = 0; i < hasWorkFlags.size() && !bAnyWork; ++i)
	{
		bAnyWork = bAnyWork || hasWorkFlags[i];
	}
	hasWorkMutex.unlock();

	if (bAnyWork)
	{
		std::unique_lock<std::mutex> cvLock(hasWorkMutex);
		noWorkCondVar.wait(cvLock);
	}
}

void ThreadPool::WaitForWorkersTermination()
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

void ThreadPool::Internal_SetWorkerHasWork(int32 workerIndex, bool bHasWork)
{
	hasWorkMutex.lock();
	if (hasWorkFlags[workerIndex] != bHasWork)
	{
		hasWorkFlags[workerIndex] = bHasWork;

		bool bAnyWork = false;
		for (size_t i = 0; i < hasWorkFlags.size() && !bAnyWork; ++i)
		{
			bAnyWork = bAnyWork || hasWorkFlags[i];
		}
		if (!bAnyWork)
		{
			noWorkCondVar.notify_all();
		}
	}
	hasWorkMutex.unlock();
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
