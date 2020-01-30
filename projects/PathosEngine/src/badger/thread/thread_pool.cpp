#include "thread_pool.h"
#include "badger/assertion/assertion.h"

static void* PooledThreadMain(void* _param)
{
	PooledThreadParam* param = reinterpret_cast<PooledThreadParam*>(_param);
	int32 threadID           = param->threadID;
	ThreadPool* pool         = param->pool;

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

void ThreadPool::StartService(uint32 numWorkerThreads)
{
	CHECK(state == ThreadPoolState::NotStarted);
	CHECK(numWorkerThreads <= 256); // you sure it's not underflowed?

	state = ThreadPoolState::Active;

	threads.resize(numWorkerThreads);
	threadParams.resize(numWorkerThreads);

	for (int32 i = 0; i < numWorkerThreads; ++i)
	{
		threadParams[i].threadID = i;
		threadParams[i].pool = this;
	}

	for (int32 i = 0; i < numWorkerThreads; ++i)
	{
		threads[i] = std::thread(PooledThreadMain, (void*)&threadParams[i]);
	}
}

void ThreadPool::StopService()
{
	if (state != ThreadPoolState::Active)
	{
		return;
	}

	state = ThreadPoolState::PendingKill;

	cond_var.notify_all();
	WaitForActiveWorks();

	state = ThreadPoolState::Destroyed;
}

void ThreadPool::WaitForActiveWorks()
{
}

void ThreadPool::WaitForAllWorks()
{
	if (state != ThreadPoolState::Active)
	{
		return;
	}

	int32 n = (int32)threads.size();
	for (int32 i = 0; i < n; ++i)
	{
		threads[i].join();
	}
}

void ThreadPool::AddWorkUnsafe(const ThreadPoolWork& workItem)
{
	queue.push(workItem);
}

void ThreadPool::AddWorkSafe(const ThreadPoolWork& workItem)
{
	queueLock.lock();

	queue.push(workItem);

	queueLock.unlock();
}

bool ThreadPool::Internal_PopWork(ThreadPoolWork& work)
{
	bool ret = true;

	queueLock.lock();

	if (queue.empty())
	{
		return false;
	}
	else
	{
		work = queue.front();
		queue.pop();
	}

	queueLock.unlock();

	return ret;
}
