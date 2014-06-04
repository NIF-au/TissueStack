#include "execution.h"
#include "networking.h"

int main(int argc, char * args[])
{
	tissuestack::execution::ThreadPool thread_pool(10);
	thread_pool.init();
	std::this_thread::sleep_for(std::chrono::seconds(10));
	thread_pool.testNotify();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	thread_pool.testNotify();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	thread_pool.testNotify();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	thread_pool.testNotify();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	thread_pool.stop();
	std::this_thread::sleep_for(std::chrono::seconds(5));

	/*
	tissuestack::execution::SharedLibraryFunctionCall so("/tmp/test.so");
	so.init();

	const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> f =
		  [&so] (const tissuestack::common::ProcessingStrategy * _this)
		  {
			const void * dlSymReturn = so.callDlSym(std::string("lala"));
			// check return
			if (dlSymReturn == nullptr) return;

			// cast to desired type
			int (*test)(const char *) = (int (*) (const char *)) dlSymReturn;


			std::string t("Hello World");
			std::cout << "Before Call: " << t << std::endl;
			test(t.c_str());
			std::cout << "After  Call: " << t << std::endl;
		  };

	so.process(&f);
	so.stop();
	*/
	return 1;
}
