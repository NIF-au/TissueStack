#ifndef	__EXECUTION_H__
#define __EXECUTION_H__

#include "tissuestack.h"
#include "imaging.h"
#include <condition_variable>
#include <thread>
#include <chrono>
#include <dlfcn.h>
#include <functional>
#include <cmath>
#include <errno.h>

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
				void addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * removeTask();
				bool hasNoTasksQueued();
				void stop();

			private:
				std::mutex _task_queue_mutex;
				std::mutex _conditional_mutex;
				std::condition_variable _notification_condition;
				short _number_of_threads = 0;
				WorkerThread ** _workers = nullptr;
				std::queue<const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> *> _work_load;
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

		class TissueStackOnlineExecutor final
		{
			public:
				TissueStackOnlineExecutor & operator=(const TissueStackOnlineExecutor&) = delete;
				TissueStackOnlineExecutor(const TissueStackOnlineExecutor&) = delete;
				static TissueStackOnlineExecutor * instance();
				void execute(std::string request, int client_descriptor);
				std::string composeHttpResponse(std::string status, std::string content_type, std::string content);
				~TissueStackOnlineExecutor();
			private:
				TissueStackOnlineExecutor();
				tissuestack::common::RequestFilter ** _filters = nullptr;
				tissuestack::imaging::ImageExtraction<tissuestack::imaging::SimpleCacheHeuristics> * _imageExtractor = nullptr;
				static TissueStackOnlineExecutor * _instance;

		};
	}
}

#endif	/* __EXECUTION_H__ */
