/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef	__EXECUTION_H__
#define __EXECUTION_H__

#include "tissuestack.h"
#include <condition_variable>
#include <time.h>
#include <thread>
#include <chrono>
#include <dlfcn.h>
#include <functional>
#include <cmath>
#include <errno.h>

namespace tissuestack
{
	namespace imaging
	{
		class SimpleCacheHeuristics; // forward declarations
		template <typename CachingStrategy>
		class ImageExtraction;
		class PreTiler;
		class RawConverter;
	}
	namespace services
	{
	 class TissueStackTask; // forward declarations
	 class TissueStackConversionTask; // forward declarations
	 class TissueStackServicesDelegator;
	}
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
				virtual void init();
				virtual void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				virtual void addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				virtual const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * removeTask();
				virtual bool hasNoTasksQueued();
				void stop();
			protected:
				void init0(std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> wait_loop);
			private:
				std::mutex _task_queue_mutex;
				short _number_of_threads = 0;
				WorkerThread ** _workers = nullptr;
				std::queue<const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> *> _work_load;
		};

		class TissueStackTaskQueueExecutor: public ThreadPool
		{
			public:
				TissueStackTaskQueueExecutor & operator=(const TissueStackTaskQueueExecutor&) = delete;
				TissueStackTaskQueueExecutor(const TissueStackTaskQueueExecutor&) = delete;
				explicit TissueStackTaskQueueExecutor();
				void init();
				void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				void addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality);
				const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * removeTask();
				bool hasNoTasksQueued();
			private:
				std::mutex _conditional_mutex;
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
				void execute(
					const tissuestack::common::ProcessingStrategy * processing_strategy,
					const std::string request,
					int client_descriptor);
				void executeTask(
					const tissuestack::common::ProcessingStrategy * processing_strategy,
					const tissuestack::services::TissueStackTask * task);
				~TissueStackOnlineExecutor();
			private:
				TissueStackOnlineExecutor();
				tissuestack::common::RequestFilter ** _filters = nullptr;
				tissuestack::imaging::ImageExtraction<tissuestack::imaging::SimpleCacheHeuristics> * _imageExtractor = nullptr;
				tissuestack::services::TissueStackServicesDelegator * _serviesDelegator = nullptr;
				tissuestack::imaging::RawConverter * _tissueStackRawConverter = nullptr;
				tissuestack::imaging::PreTiler * _tissueStackPreTiler = nullptr;
				static TissueStackOnlineExecutor * _instance;

		};

		class TissueStackOfflineExecutor final : public tissuestack::common::ProcessingStrategy
		{
			public:
				TissueStackOfflineExecutor & operator=(const TissueStackOfflineExecutor&) = delete;
				TissueStackOfflineExecutor(const TissueStackOfflineExecutor&) = delete;
				static TissueStackOfflineExecutor * instance();

				void preTile(
					const std::string & in_file,
					const std::string & tile_dir,
					const std::vector<std::string> & dimensions,
					const std::vector<unsigned short> & zoom_levels,
					const std::string & colormap);

				void convert(
					const tissuestack::services::TissueStackConversionTask * task,
					const std::string dimension = "",
					const bool writeHeader = true);

				void init();
				void process(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality) ;
				void stop();
				~TissueStackOfflineExecutor();
			private:
				static const unsigned short SHUTDOWN_TIMEOUT_IN_SECONDS = 15;
				TissueStackOfflineExecutor();
				tissuestack::imaging::RawConverter * _tissueStackRawConverter = nullptr;
				tissuestack::imaging::PreTiler * _tissueStackPreTiler = nullptr;
				static TissueStackOfflineExecutor * _instance;
		};

	}
}

#endif	/* __EXECUTION_H__ */
