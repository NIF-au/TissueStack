#ifndef	__POOL_H__
#define __POOL_H__

#include "tissuestack.h"
#include <thread>

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
	}
}

#endif	/* __POOL_H__ */
