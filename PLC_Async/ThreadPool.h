#pragma once
#include <future>
#include <functional>

class CThreadPool {
public:

	CThreadPool( size_t num_of_threads = 1 );
	~CThreadPool() {};

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
	std::shared_ptr<std::future<void>> runAsync2( FN fn, ARGS... args )
	{
		auto p = getFreeWorker();
		auto f = std::make_shared<std::future<void>>( p->get_future() );

		std::function<void()> rfn = std::bind( fn, args... );

		std::thread( [=]( std::shared_ptr<std::promise<void>> p ) {
			rfn();
			p->set_value();
		}, p ).detach();

		return f;
	}

	std::shared_ptr<std::promise<void>> GetFreeWorker();

private:
	std::shared_ptr<std::promise<void>> getFreeWorker();

	std::vector<std::shared_ptr<std::promise<void>>> workers;
	std::vector<bool> workersFree;
};