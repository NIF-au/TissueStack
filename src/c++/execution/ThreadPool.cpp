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
#include "execution.h"

tissuestack::execution::ThreadPool::ThreadPool(short number_of_threads) :
	_number_of_threads(number_of_threads)
{
	if (number_of_threads <1)
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
				//std::this_thread::sleep_for(std::chrono::milliseconds(10));
				usleep(10000); // 10,000 micro seconds /10 milli seconds

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
	this->init0(wait_loop);
}

void tissuestack::execution::ThreadPool::init0(std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> wait_loop)
{
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
		this->addTask(functionality);
}

void tissuestack::execution::ThreadPool::stop()
{
	// raise stop flag to prevent new requests from being processed
	if (this->isRunning() && !this->isStopFlagRaised())
		this->raiseStopFlag();

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
