#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <future>
#include <chrono>
#include <ctime>
#include "ThreadPool.h"

static void print( const std::string& msg )
{
	std::cout << msg << '\n';
}
static void print( const int msg )
{
	std::cout << msg << '\n';
}
static void time( const char end = ' ' )
{
	std::cout << __TIMESTAMP__ << end;
}

void threadsFun()
{
	auto func = []( const std::string& str )
	{
		std::cout << "thread_id = " << std::this_thread::get_id() << '\n';
	};

	std::vector<std::thread> v;
	int num_of_threads = 1000;
	for( int i = 0; i < num_of_threads; i++ ) {
		v.push_back( std::thread( func, std::to_string( i ) ) );
	}
	for( int i = num_of_threads - 1; i >= 0; i-- ) {
		v[i].join();
	}

}

void std_async()
{
	std::cout << "Main thread id=" << std::this_thread::get_id() << "\n";
	auto asyncDefault = std::async( []()
	{
		std::cout << "Async default, Threadid=" <<
			std::this_thread::get_id() << "\n";
	} );

	auto asyncDeffered = std::async( std::launch::deferred,
		[]( const std::string& str )
	{
		std::cout << "Async deffer, Threadid="
			<< std::this_thread::get_id() << "," << str << "\n";
	}, std::string( "end string" ) );

	auto asyncDeffered2 = std::async( std::launch::deferred, []()
	{
		std::cout << "Async deffer2, Threadid="
			<< std::this_thread::get_id() << "\n";
	} );

	auto trueAsync = std::async( std::launch::async, []()
	{
		std::cout << "True async, Threadid="
			<< std::this_thread::get_id() << "\n";
	} );
	std::cout << "Sleep started\n";
	std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
	std::cout << "Sleep ended\n";
	asyncDefault.get();
	asyncDeffered.get();
	trueAsync.get();
}

void threadId()
{
	std::cout << "This thread id = " << std::this_thread::get_id() << "\n";
}

void trashFun()
{
	std::cout << "This thread id = " << std::this_thread::get_id() << "\n";
	std::cout << "666trashFun\n";
}

int foo()
{
	return 666;
}

int sum( int a, int b )
{
	return a + b;
}

void wtf( int a = 0 )
{
	threadId();
	int res = 0;
	for( int i = 0; i < a; i++ ) {
		for( int j = 0; j < a; j++ ) {
			res += j - i;
		}
	}
	res += 666;
	std::cout << res << "wtf\n";
	//return res;
}

void message( const std::string& msg )
{
	std::cout << msg << '\n';
}


void fun()
{

	std::cout << "MainThreadid = " << std::this_thread::get_id() << '\n';
	auto spPromise = std::make_shared<std::promise<void>>();
	auto spWaiter = std::make_shared<std::future<void>>( spPromise->get_future() );
	auto copySpWaiter = spWaiter;

	auto call = [spPromise]( size_t value )
	{
		time( '\n' );
		std::cout << "Threadid = " << std::this_thread::get_id() << " start work \n";
		size_t i = std::numeric_limits<size_t>::max();
		while( i-- ) {
			if( i == value )
				spPromise->set_value();
		}
		time( '\n' );
		std::cout << "Threadid = " << std::this_thread::get_id() << " finish work \n";
	};

	print( "VZ 0" );
	std::thread thread( call, std::numeric_limits<size_t>::max() - 500 );
	thread.detach();
	print( "VZ 1" );
	spWaiter.get();
	spWaiter.get();
	print( "VZ 2" );
	copySpWaiter.get();
	print( "VZ 3" );

	//std::future<void> waiter = spPromise->get_future();

	//std::function<std::string( int, int )> myFn = [&]( int a, int b )
	//{
	//	std::string str = "String = " + std::to_string( a ) + " " + std::to_string( b );
	//	return str;
	//};

	//auto r1 = pool.runAsync<double>( &foo );
	//auto r2 = pool.runAsync<int>( &sum, 100, 54 );
	//auto r3 = pool.runAsync<bool>( []() { return true; } );
	//auto r4 = pool.runAsync<std::string>( myFn, 50, 20 );

	//while( !r1->ready );  // ∆дем результатов
	//std::cout << r1->data << std::endl;
	//while( !r2->ready );
	//std::cout << r2->data << std::endl;
	//while( !r3->ready );
	//std::cout << r3->data << std::endl;
	//while( !r4->ready );
	//std::cout << r4->data << std::endl;
}

void notFun()
{
	std::cout << "MainThreadid = " << std::this_thread::get_id() << '\n';
	CThreadPool threadPool( 1 );

	auto f0 = threadPool.runAsync2( false, &wtf, 1000 );
	auto f1 = threadPool.runAsync2( false, &trashFun );
	auto f2 = threadPool.runAsync2( false, &trashFun );
	auto f3 = threadPool.runAsync2( false, &trashFun );
	auto f4 = threadPool.runAsync2( false, &wtf, 1000 );
	auto f5 = threadPool.runAsync2( false, &trashFun );
	auto f6 = threadPool.runAsync2( false, &trashFun );
	auto f7 = threadPool.runAsync2( false, &trashFun );
	//auto f4 = threadPool.runAsync2( false, &message, "lol" );
	f0.get();
	f1.get();
	f2.get();
	f3.get();
	f4.get();
	f5.get();
	f6.get();
	f7.get();
	//f4.get();
	threadPool.WaitForFinishWork();
}

void ass()
{
	threadId();
	std::promise<int> promise;
	std::shared_ptr<std::future<int>> f = std::make_shared<std::future<int>>( promise.get_future() );
	std::thread( []( std::promise<int>& p ) {
		auto res = foo();
		threadId();
		p.set_value( res );
	}, std::ref( promise ) ).detach();

	std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
	std::cout << f.get() << '\n';
}

int main()
{
	//ass();
	//std::this_thread::sleep_for( std::chrono::seconds( 3 ) );
	//future.get();
	//fun();

	notFun();

	//std_async();
	system( "pause" );
}