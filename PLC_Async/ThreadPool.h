#pragma once
#include <future>
#include <queue>
#include <functional>

typedef std::function<void()> fn_type;
typedef std::shared_ptr<std::promise<void>> promise_ptr;
typedef std::shared_ptr<std::future<void>> future_ptr;

class CWorker {
public:
	CWorker( int curWorker = 0 ) : enabled( true ), fqueue(), 
		spPromise( std::make_shared<std::promise<void>>( std::promise<void>() ) ),
		spFuture( std::make_shared<std::future<void>>( spPromise->get_future() ) ),
		currentWorker( curWorker ), thread( &CWorker::thread_fn, this )
	{
	}

	~CWorker()
	{
		enabled = false;
		cv.notify_one();
		thread.join();
		while( !fqueue.empty() ) {
			fqueue.pop();
		}
	}

	void AppendFn( fn_type fn )
	{
		std::unique_lock<std::mutex> locker( mutex );
		fqueue.push( fn );
		cv.notify_one();
	}

	size_t GetTaskCount()
	{
		std::unique_lock<std::mutex> locker( mutex );
		return fqueue.size();
	}

	bool isEmpty()
	{
		std::unique_lock<std::mutex> locker( mutex );
		return fqueue.empty();
	}

	promise_ptr GetPromise()
	{
		return spPromise;
	}

	future_ptr GetFuture()
	{
		return spFuture;
	}

private:
	bool enabled;
	std::condition_variable cv;
	std::queue<fn_type> fqueue;
	std::mutex mutex;
	std::thread thread;
	int currentWorker;
	promise_ptr spPromise;
	future_ptr spFuture;

	void thread_fn()
	{
		while( enabled ) {
			std::unique_lock<std::mutex> locker( mutex );
			cv.wait( locker, [&]() { return !fqueue.empty() || !enabled; } );
			while( !fqueue.empty() ) {
				fn_type fn = fqueue.front();
				fqueue.pop();
				locker.unlock();
				fn();
				locker.lock();
				//fqueue.pop();
			}
		}
	}
};

typedef std::shared_ptr<CWorker> worker_ptr;

class CThreadPool {
public:

	CThreadPool( size_t num_of_threads = 1 );
	~CThreadPool();

	template<class FN, class... ARGS>
	void runAsync( FN fn, ARGS... args )
	{

		//std::promise<R> p;
		//std::future<R> f = p.get_future();
		//std::thread( []( std::promise<R>& p ) { p.set_value(); },
		//	std::ref( p ) ).detach();
		//std::cout << f.get() << '\n';

		//std::promise<void> p; std::future<void> f = p.get_future();
		
		//auto p = workers[0]; 
		auto p = getFreeWorker();
		auto f = p->get_future();

		std::function<void()> rfn = std::bind( fn, args... );

		std::thread( [=]( std::shared_ptr<std::promise<void>> p ) {
			rfn();
			p->set_value();
			//p.set_value();
		}, p ).detach();
		f.get();

		//std::cout << f.get() << '\n';

		//		auto p = getFreeWorker();
		//		//std::promise<void> p;
		//		auto f = std::make_shared<std::future<void>>( p->get_future() );
		//
		//		//std::future<void> f = p.get_future();
		//
		////		std::thread thread( [&p] { auto result = std::bind( fn, args... ); p->set_value( result ); } );
		//		std::thread( []( std::promise<void>& p ) {
		//			p.set_value(); 
		//		}, std::ref( p ) ).detach();

		//		return f;
	}

	//std::shared_ptr<std::shared_future<void>> runAsync2( bool syncMode, FN fn, ARGS... args )
	//std::shared_ptr<std::future<void>> runAsync2( bool syncMode, FN fn, ARGS... args )
	template<class FN, class... ARGS>
	std::shared_ptr<std::future<void>> runAsync2( bool syncMode, FN fn, ARGS... args )
	{
		if( hasFreeWorker() && !syncMode ) {
			int i = getFreeWorkerIndex();
			setWorkerBusy( i );
			auto p = workers[i];
			//auto f = futures[i];
			//auto f = std::move( futures[i] );
			//auto p = getFreeWorker();
			//auto ff = p->get_future();
			auto f = std::make_shared<std::future<void>>( std::move( p->get_future() ) );
			//auto f = std::make_shared<std::future<void>>( p->get_future() );
			//auto f = std::make_shared<std::shared_future<void>>( p->get_future().share() );

			std::function<void()> rfn = std::bind( fn, args... );

			std::thread( [=]( std::shared_ptr<std::promise<void>> p, int i ) {
				rfn();
				setWorkerFree( i );
				p->set_value();
			}, p, i ).detach();
			return f;
		} else {
			auto p = std::make_shared<std::promise<void>>();
			auto f = std::make_shared<std::future<void>>( p->get_future() );
			//auto f = std::make_shared<std::shared_future<void>>( p->get_future().share() );
			std::function<void()> rfn = std::bind( fn, args... );
			rfn();
			p->set_value();
			return f;
		}
	}

	template<class FN, class... ARGS>
	std::shared_ptr<std::future<void>> runAsync3( bool syncMode, FN fn, ARGS... args )
	{
		if( hasFreeWorker() && !syncMode ) {
			int i = getFreeWorkerIndex();
			setWorkerBusy( i );
			auto p = workers[i];
			auto f = futures[i];

			std::function<void()> rfn = std::bind( fn, args... );
			//std::function<void()> 
			auto fn_wrapper = [p, i, rfn, this]() {
				rfn();
				//setWorkerFree( i );
				//workersFree[i] = true;
				this->SetWorkerFree( i );
				p->set_value();
			}; // , p, i;
			//fn_wrapper( p, i );
			fn_wrapper();

			//std::thread( [=]( std::shared_ptr<std::promise<void>> p, int i ) {
			//	rfn();
			//	setWorkerFree( i );
			//	p->set_value();
			//}, p, i ).detach();
			std::thread( fn_wrapper ).detach();

			return f;
		} else {
			auto p = std::make_shared<std::promise<void>>();
			auto f = std::make_shared<std::future<void>>( p->get_future() );
			//auto f = std::make_shared<std::shared_future<void>>( p->get_future().share() );
			std::function<void()> rfn = std::bind( fn, args... );
			rfn();
			p->set_value();
			return f;
		}
	}

	template<class FN, class... ARGS>
	std::shared_ptr<std::future<void>> runAsync4( bool syncMode, FN fn, ARGS... args )
	{
		if( spWorker->isEmpty() ) {
			auto p = spWorker->GetPromise();
			auto f = spWorker->GetFuture();
			std::function<void()> rfn = std::bind( fn, args... );
			//std::function<void()> 
			auto fn_wrapper = [p, rfn]() {
				rfn();
				//setWorkerFree( i );
				//workersFree[i] = true;
				//this->SetWorkerFree( i );
				p->set_value();
				//p->set_value_at_thread_exit();
			};
			//fn_wrapper();
			spWorker->AppendFn( fn_wrapper );
			return f;
		} else {
			auto p = std::make_shared<std::promise<void>>();
			auto f = std::make_shared<std::future<void>>( p->get_future() );
			//auto f = std::make_shared<std::shared_future<void>>( p->get_future().share() );
			std::function<void()> rfn = std::bind( fn, args... );
			rfn();
			p->set_value();
			return f;
		}
	}

	std::shared_ptr<std::promise<void>> GetFreeWorker();
	void WaitForFinishWork( int numOfIterations = 50000 );

	void WaitForFinishWork4( int numOfIterations = 50000 )
	{
		waitForFinishWork4( numOfIterations );
	}

	void SetWorkerFree( int i )
	{
		setWorkerFree( i );
	}

private:

	std::shared_ptr<std::promise<void>> getFreeWorker();
	int getFreeWorkerIndex();
	bool hasFreeWorker();
	void setWorkerBusy( int i );
	void setWorkerFree( int i );

	void waitForFinishWork( int numOfIterations = 50000 );
	void waitForFinishWork4( int numOfIterations = 50000 );

	std::vector<std::shared_ptr<std::promise<void>>> workers;
	std::vector<std::shared_ptr<std::future<void>>> futures;
	std::vector<bool> workersFree;

	worker_ptr spWorker;
};