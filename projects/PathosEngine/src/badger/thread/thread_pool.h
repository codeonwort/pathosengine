#pragma once

#include "badger/types/int_types.h"

#include <mutex>
#include <queue>
#include <atomic>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>

class ThreadPool;
struct WorkItemParam;

using WorkItemRoutine = std::function<void(const WorkItemParam*)>;

struct ThreadPoolWork
{
	WorkItemRoutine routine;
	void* arg;
};

// Parameter for threads in the thread pool
struct PooledThreadParam
{
	PooledThreadParam()
		: threadID(-1)
		, pool(nullptr)
		, working(false)
	{
	}

	int32             threadID;
	ThreadPool*       pool;
	bool              working;
};

// Passed to the WorkItemRoutine as a sole parameter
struct WorkItemParam
{
	int32 threadID;
	void* arg;
};

enum class ThreadPoolState
{
	NotStarted,
	Active,
	PendingKill,
	Destroyed
};

class ThreadPool final
{

public:
	ThreadPool()
		: state(ThreadPoolState::NotStarted)
	{
	}

	~ThreadPool()
	{
	}

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	// Create worker threads.
	void Start(uint32 numWorkerThreads);

	// Discard pending works and destroy this thread pool. Call WaitForAllWorks() first if you want to process all pending works.
	void Stop();

	// #todo-thread-pool: Implement a mechanism to cancel active works.
	//void CancelActiveWorks();

	// [Blocking operation] Waits for active works to finish.
	//void WaitForActiveWorks();

	// [Blocking operation] Waits for all works to finish.
	void WaitForAllWorks();

	// OK to call this before Start() to avoid redundant lock/unlock. Never use this after Start().
	void AddWorkUnsafe(const ThreadPoolWork& workItem);

	// OK to call anytime, but you must use this after Start().
	void AddWorkSafe(const ThreadPoolWork& workItem);

	// CAUTION: Do not call directly. This is public just for worker threads.
	bool Internal_PopWork(ThreadPoolWork& work);

public:
	std::vector<std::thread>                 threads;
	std::vector<PooledThreadParam>           threadParams;

	std::mutex                               worker_mutex;
	std::condition_variable                  cond_var;

	std::queue<ThreadPoolWork>               queue;
	std::mutex                               queueLock;

	ThreadPoolState                          state;
};
