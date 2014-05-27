#ifndef	__POOL_H__
#define __POOL_H__

#include "tissuestack.h"
#include <thread>
#include <dlfcn.h>
#include <functional>

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
				void process(const std::function<void (const tissuestack::common::Request * request, tissuestack::common::ProcessingStrategy * _this)> * functionality,
						const tissuestack::common::Request * request);
				void stop();

			private:
				short _number_of_threads;
		};

		class SimpleSequentialExecution: public tissuestack::common::ProcessingStrategy
		{
			public:
				SimpleSequentialExecution & operator=(const SimpleSequentialExecution&) = delete;
				SimpleSequentialExecution(const SimpleSequentialExecution&) = delete;
				SimpleSequentialExecution();
				void init();
				void process(const std::function<void (const tissuestack::common::Request * request, tissuestack::common::ProcessingStrategy * _this)> * functionality,
						const tissuestack::common::Request * request);
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
				void process(const std::function<void (const tissuestack::common::Request * request, tissuestack::common::ProcessingStrategy * _this)> * functionality,
						const tissuestack::common::Request * request);
				void stop();
				void * const callDlSym(std::string function_name);
			private:
				const std::string _so_library_path;
				void * _so_handle;
		};
	}
}

#endif	/* __POOL_H__ */
