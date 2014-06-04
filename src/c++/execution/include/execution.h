#ifndef	__EXECUTION_H__
#define __EXECUTION_H__

#include "tissuestack.h"
#include <condition_variable>
#include <thread>
#include <chrono>
#include <dlfcn.h>
#include <functional>

namespace tissuestack
{
	namespace execution
	{
		class WorkerThread : public std::thread
		{
			public:
				explicit WorkerThread(
						std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> wait_loop);
				bool isRunning() const;
				void stop();
			private:
				bool _is_running = false;
		};

		class ThreadPool: public tissuestack::common::ProcessingStrategy
		{
			public:
				ThreadPool & operator=(const ThreadPool&) = delete;
				ThreadPool(const ThreadPool&) = delete;
				explicit ThreadPool(short number_of_threads);
				~ThreadPool();
				short getNumberOfThreads() const;
				void init();
				void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				void stop();

			private:
				std::mutex _conditional_mutex;
				std::condition_variable _notification_condition;
				short _number_of_threads = 0;
				WorkerThread ** _workers = nullptr;
		};

		class SimpleSequentialExecution: public tissuestack::common::ProcessingStrategy
		{
			public:
				SimpleSequentialExecution & operator=(const SimpleSequentialExecution&) = delete;
				SimpleSequentialExecution(const SimpleSequentialExecution&) = delete;
				SimpleSequentialExecution();
				void init();
				void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
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
				void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				void stop();
				void * const callDlSym(std::string function_name);
			private:
				const std::string _so_library_path;
				void * _so_handle;
		};
	}
}

#endif	/* __EXECUTION_H__ */
