// Copyright 2018, Institute for Artificial Intelligence - University of Bremen
// Author: Andrei Haidu (http://haidu.eu)

#include "Async/AsyncWork.h"

/**
* Worker to save raw data
*/
class SLRawDataAsyncWorker : public FNonAbandonableTask
{
	friend class FAsyncTask<SLRawDataAsyncWorker>;

	UWorld* World;

	SLRawDataAsyncWorker(UWorld* InWorld) : World(InWorld)
	{
	}

	void DoWork()
	{
		UE_LOG(LogTemp, Error, TEXT("[%s][%d] Task done World name=%s"), TEXT(__FUNCTION__), __LINE__, *World->GetName());
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(SLRawDataAsyncWorker, STATGROUP_ThreadPoolAsyncTasks);
	}
};

/**
FAsyncTask - template task for jobs queued to thread pools

Sample code:

class ExampleAsyncTask : public FNonAbandonableTask
{
friend class FAsyncTask<ExampleAsyncTask>;

int32 ExampleData;

ExampleAsyncTask(int32 InExampleData)
: ExampleData(InExampleData)
{
}

void DoWork()
{
... do the work here
}

FORCEINLINE TStatId GetStatId() const
{
RETURN_QUICK_DECLARE_CYCLE_STAT(ExampleAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
}
};

void Example()
{

//start an example job

FAsyncTask<ExampleAsyncTask>* MyTask = new FAsyncTask<ExampleAsyncTask>( 5 );
MyTask->StartBackgroundTask();

//--or --

MyTask->StartSynchronousTask();

//to just do it now on this thread
//Check if the task is done :

if (MyTask->IsDone())
{
}

//Spinning on IsDone is not acceptable( see EnsureCompletion ), but it is ok to check once a frame.
//Ensure the task is done, doing the task on the current thread if it has not been started, waiting until completion in all cases.

MyTask->EnsureCompletion();
delete Task;
}
**/