#include <cassert>
#include <iostream>
#include "ThreadPool.h"

CThreadPool::CThreadPool( size_t num_of_threads )
{
	for( int i = 0; i < num_of_threads; i++ ) {
		auto worker = std::make_shared<std::promise<void>>();
		workers.push_back( worker );
		workersFree.push_back( true );
	}
}

std::shared_ptr<std::promise<void>> CThreadPool::GetFreeWorker()
{
	return getFreeWorker();
}

std::shared_ptr<std::promise<void>> CThreadPool::getFreeWorker()
{
	for( int i = 0; i < workersFree.size(); i++ ) {
		if( workersFree[i] ) {
			workersFree[i] = false;
			return workers[i];
		}
	}
}
