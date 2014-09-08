#include "networking.h"
#include "imaging.h"
#include "services.h"
#include "execution.h"

tissuestack::execution::TissueStackTaskQueueExecutor::TissueStackTaskQueueExecutor() :
	tissuestack::execution::ThreadPool(1)
{
	tissuestack::logging::TissueStackLogger::instance()->info("Launching Task Queue Executor");
}

void tissuestack::execution::TissueStackTaskQueueExecutor::init()
{
	// the wait loop
	std::function<void (tissuestack::execution::WorkerThread * assigned_worker)> wait_loop =
		[this] (tissuestack::execution::WorkerThread * assigned_worker)
		{
		tissuestack::logging::TissueStackLogger::instance()->info(
				"Task Queue Thread %u is ready\n",
				std::hash<std::thread::id>()(std::this_thread::get_id()));

			while (!this->isStopFlagRaised())
			{
				std::unique_lock<std::mutex> lock_on_conditional_mutex(this->_conditional_mutex);
				this->_notification_condition.wait_for(lock_on_conditional_mutex, std::chrono::milliseconds(100));

				if (tissuestack::services::TissueStackTaskQueue::instance() == nullptr)
					continue;

				// fetch next item from the globale task queue
				tissuestack::services::TissueStackTask * next_task =
					const_cast<tissuestack::services::TissueStackTask *>(
					tissuestack::services::TissueStackTaskQueue::instance()->getNextTask());
				if (next_task == nullptr) continue;

				// delegate to online executor
				tissuestack::execution::TissueStackOnlineExecutor::instance()->executeTask(
					this, next_task);
			}
			tissuestack::logging::TissueStackLogger::instance()->info(
					"Task Queue Thread %u is about to stop working!\n",
					std::hash<std::thread::id>()(std::this_thread::get_id()));
			assigned_worker->stop();
		};
	this->init0(wait_loop);
}

void tissuestack::execution::TissueStackTaskQueueExecutor::process(
		const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// this does not nothing, we run all the time and take our input from the global task queue
}

void tissuestack::execution::TissueStackTaskQueueExecutor::addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	// this does not nothing, we run all the time and take our input from the global task queue
}

const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * tissuestack::execution::TissueStackTaskQueueExecutor::removeTask()
{
	// this does not nothing, we run all the time and take our input from the global task queue
	return nullptr;
}

bool tissuestack::execution::TissueStackTaskQueueExecutor::hasNoTasksQueued()
{
	if (tissuestack::services::TissueStackTaskQueue::instance() == nullptr) return true;

	return tissuestack::services::TissueStackTaskQueue::instance()->getNextTask() == nullptr;
}

