#pragma once
#include <future>
#include <functional>

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

	template<class FN, class... ARGS>
	std::shared_ptr<std::future<void>> runAsync2( bool syncMode, FN fn, ARGS... args )
	{
		if( hasFreeWorker() && !syncMode ) {
			int i = getFreeWorkerIndex();
			setWorkerBusy( i );
			auto p = workers[i];
			//auto p = getFreeWorker();
			//p->~promise();
			//auto ff = p->get_future();
			auto f = std::make_shared<std::future<void>>( std::move( p->get_future() ) );

			//auto f = std::make_shared<std::future<void>>( p->get_future() );

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
			std::function<void()> rfn = std::bind( fn, args... );
			rfn();
			p->set_value();
			return f;
		}
	}

	std::shared_ptr<std::promise<void>> GetFreeWorker();
	void WaitForFinishWork( int timeout = 50000 );
private:
	std::shared_ptr<std::promise<void>> getFreeWorker();
	int getFreeWorkerIndex();
	bool hasFreeWorker();
	void setWorkerBusy( int i );
	void setWorkerFree( int i );
	void waitForFinishWork( int timeout = 50000 );
	std::vector<std::shared_ptr<std::promise<void>>> workers;
	std::vector<bool> workersFree;
};