#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <Windows.h>

static void print( const std::string& msg )
{
	std::cout << msg << '\n';
}
static void print( const int msg )
{
	std::cout << msg << '\n';
}

void fun()
{
	auto func = []( const std::string& str )
	{
		print( str );
		::GetCurrentThreadId();
		print( ::GetCurrentThreadId() );
	};

	std::vector<std::thread> v;
	int num_of_threads = 1000;
	for( int i = 0; i < num_of_threads; i++ ) {
		v.push_back( std::thread( func, std::to_string(i) ) );
	}
	for( int i = num_of_threads - 1; i >= 0; i-- ) {
		v[i].join();
	}

}

int main()
{
	fun();

	system( "pause" );
}