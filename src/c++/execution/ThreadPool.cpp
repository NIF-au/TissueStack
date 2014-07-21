#include "execution.h"

tissuestack::execution::ThreadPool::ThreadPool(short number_of_threads) :
	_number_of_threads(number_of_threads)
{
	if (number_of_threads <=1)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException, "A Thread Pool with less than 1 threads is not of much use!");
	this->_workers = new tissuestack::execution::WorkerThread*[number_of_threads];
	tissuestack::logging::TissueStackLogger::instance()->info("Thread Pool Size: %u\n", number_of_threads);
}

tissuestack::execution::ThreadPool::~ThreadPool()
{
	int i=0;
	while (i<this->_number_of_threads) {
		if (this->_workers[i]) delete this->_workers[i];
		i++;
	}

	delete [] this->_workers;
}

short tissuestack::execution::ThreadPool::getNumberOfThreads() const
{
	return this->_number_of_threads;
}

void tissuestack::execution::ThreadPool::init()
{
	// the wait loop
	std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> wait_loop =
		[this] (tissuestack::execution::WorkerThread * assigned_worker)
		{
		tissuestack::logging::TissueStackLogger::instance()->info(
				"Thread %u is ready\n",
				std::hash<std::thread::id>()(std::this_thread::get_id()));

			while (!this->isStopFlagRaised())
			{
				std::unique_lock<std::mutex> lock_on_conditional_mutex(this->_conditional_mutex);
				this->_notification_condition.wait(lock_on_conditional_mutex);

				// fetch next item from the queue if not empty
				if (this->hasNoTasksQueued())
					continue;

				const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * next_task = this->removeTask();
				if (next_task)
				{
					try
					{
						((*next_task)(this));
						// clean up pointer
						delete next_task;
					}  catch (std::exception& bad)
					{
						// clean up and propagate
						delete next_task;
						throw bad;
					}
				}
			}
			tissuestack::logging::TissueStackLogger::instance()->info(
					"Thread %u is about to stop working!\n",
					std::hash<std::thread::id>()(std::this_thread::get_id()));
			assigned_worker->stop();
		};

	// start up the threads and put them in wait mode
	int i=0;
	while (i < this->_number_of_threads)
	{
		this->_workers[i] = new tissuestack::execution::WorkerThread(wait_loop);
		this->_workers[i]->detach();
		i++;
	}

	// the thread pool is up and running
	if (!this->isStopFlagRaised())
		this->setRunningFlag(true);
}

void tissuestack::execution::ThreadPool::process(
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// dispatch functionality to the pool, only if we are running,
	// haven't received a stop flag and the closure is not null
	if (this->isRunning() && !this->isStopFlagRaised() && functionality)
	{
		this->addTask(functionality);
		this->_notification_condition.notify_one();
	}
}

void tissuestack::execution::ThreadPool::stop()
{
	// raise stop flag to prevent new requests from being processed
	if (this->isRunning() && !this->isStopFlagRaised())
	{
		this->raiseStopFlag();
		this->_notification_condition.notify_all();
	}

	// loop over all threads and check if they are down
	int i = 0;
	int numberOfThreadsRunning = 0;
	while (i < this->_number_of_threads)
	{
		if (this->_workers[i]->isRunning()) numberOfThreadsRunning++;
		i++;
	}

	if (numberOfThreadsRunning == 0)
		this->setRunningFlag(false);
}

void tissuestack::execution::ThreadPool::addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	std::lock_guard<std::mutex> lock(this->_task_queue_mutex);

	this->_work_load.push(functionality);
}

const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * tissuestack::execution::ThreadPool::removeTask()
{
	std::lock_guard<std::mutex> lock(this->_task_queue_mutex);

	if (this->_work_load.empty()) return nullptr;

	const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * ret = this->_work_load.front();
	this->_work_load.pop();

	return ret;

}

bool tissuestack::execution::ThreadPool::hasNoTasksQueued()
{
	std::lock_guard<std::mutex> lock(this->_task_queue_mutex);

	return this->_work_load.empty();
}
