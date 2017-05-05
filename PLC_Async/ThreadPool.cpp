#include <cassert>
#include <iostream>
#include <chrono>
#include <ctime>
#include "ThreadPool.h"

CThreadPool::CThreadPool( size_t num_of_threads )
{
	for( int i = 0; i < num_of_threads; i++ ) {
		auto worker = std::make_shared<std::promise<void>>();
		workers.push_back( worker );
		workersFree.push_back( true );
	}
}

CThreadPool::~CThreadPool()
{
}

void CThreadPool::WaitForFinishWork( int timeout )
{
	waitForFinishWork( timeout );
}

void CThreadPool::waitForFinishWork( int timeout )
{
	int numOfIterations = 1000;
	while( numOfIterations ) {
		bool allWorkersFree = true;
		for( int i = 0; i < workersFree.size(); i++ ) {
			if( !workersFree[i] ) {
				allWorkersFree = false;
			}
		}
		if( allWorkersFree ) {
			return;
		}
	}
	std::cout << "Some Workers work too long!\n";
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

int CThreadPool::getFreeWorkerIndex()
{
	for( int i = 0; i < workersFree.size(); i++ ) {
		if( workersFree[i] ) {
			return i;
		}
	}
}

bool CThreadPool::hasFreeWorker()
{
	bool hasFreeWorker = false;
	for( int i = 0; i < workersFree.size(); i++ ) {
		if( workersFree[i] ) {
			hasFreeWorker = true;
		}
	}
	return hasFreeWorker;
}

void CThreadPool::setWorkerBusy( int i )
{
	assert( i >= 0 );
	assert( i < workersFree.size() );
	workersFree[i] = false;
}

void CThreadPool::setWorkerFree( int i )
{
	assert( i >= 0 );
	assert( i < workersFree.size() );
	workersFree[i] = true;
}