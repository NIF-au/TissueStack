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
				std::this_thread::sleep_for(std::chrono::milliseconds(100));

				if (!tissuestack::services::TissueStackTaskQueue::doesInstanceExist())
					break;

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
	if (functionality)
		delete functionality;
}

void tissuestack::execution::TissueStackTaskQueueExecutor::addTask(const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * functionality)
{
	if (functionality)
		delete functionality;
}

const std::function<void (const tissuestack::common::ProcessingStrategy * _this)> * tissuestack::execution::TissueStackTaskQueueExecutor::removeTask()
{
	return nullptr;
}

bool tissuestack::execution::TissueStackTaskQueueExecutor::hasNoTasksQueued()
{
	if (tissuestack::services::TissueStackTaskQueue::instance() == nullptr) return true;

	return tissuestack::services::TissueStackTaskQueue::instance()->getNextTask() == nullptr;
}

