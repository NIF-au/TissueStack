#ifndef	__POOL_H__
#define __POOL_H__

#include "tissuestack.h"
#include <thread>
#include <dlfcn.h>

namespace tissuestack
{
	namespace execution
	{
		class ThreadPool: public tissuestack::common::ProcessingStrategy
		{
			public:
				ThreadPool & operator=(const ThreadPool&) = delete;
				ThreadPool(const ThreadPool&) = delete;
				explicit ThreadPool(short number_of_threads);
				short getNumberOfThreads() const;
				void init();
				void process();
				void stop();

			private:
				short _number_of_threads;
		};

		class SimpleExecution: public tissuestack::common::ProcessingStrategy
		{
			public:
				SimpleExecution & operator=(const SimpleExecution&) = delete;
				SimpleExecution(const SimpleExecution&) = delete;
				SimpleExecution();
				void init();
				void process();
				void stop();
		};

		class SharedLibraryFunctionCall: public tissuestack::common::ProcessingStrategy
		{
			public:
				SharedLibraryFunctionCall & operator=(const SharedLibraryFunctionCall&) = delete;
				SharedLibraryFunctionCall(const SharedLibraryFunctionCall&) = delete;
				explicit SharedLibraryFunctionCall(const std::string so_library_path);
				~SharedLibraryFunctionCall();
				void init();
				void process();
				void stop();
			private:
				const std::string _so_library_path;
				void * _so_handle;
		};
	}
}

#endif	/* __POOL_H__ */
